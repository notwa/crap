#define ID (0xD8D0D8D0)
#define LABEL "crap_mugi4"
#define NAME "crap mugi4 (moog-like)"
#define AUTHOR "Connor Olding"
#define COPYRIGHT "MIT"
#define PARAMETERS 3

/*
an implementation of:
S. D Angelo and V. Välimäki. Generalized Moog Ladder Filter: Part II
Explicit Nonlinear Model through a Novel Delay-Free Loop Implementation Method.
IEEE Trans. Audio, Speech, and Lang. Process.,
vol. 22, no. 12, pp. 1873 1883, December 2014.
https://aaltodoc.aalto.fi/bitstream/handle/123456789/14420/article6.pdf
*/

#define OVERSAMPLING 2
#define BLOCK_SIZE 256
#define FULL_SIZE (BLOCK_SIZE*OVERSAMPLING)

#include <stdio.h>
#include <string.h>

#include "util.hpp"
#include "param.hpp"
#include "os2piir.hpp"

#define VT 0.026
#define N 4
#define VT2 T(2.*VT)

typedef struct {
	v2df sum, sumback, dout;
} stage;

typedef struct {
	v2df g;
	v2df p0;
	v2df q0, q1, q2, q3;
	v2df r1, r2, r3, r4;
	v2df L_p0;
	v2df L_q0;
	v2df L_r1;
} freqdata;

typedef struct {
	ulong fs;
	halfband_t<v2df> hb_up, hb_down;
	freqdata fd;
	stage s1, s2, s3, s4;
	v2df sumback1, sumback2, sumback3, sumback4;
	v2df drive, feedback;
} personal;

TEMPLATE INNER PURE T
tanh2(T x)
{
	//return T(tanh(x[0]), tanh(x[1]));
	T xx = x*x;
	T a = ((xx + T(378))*xx + T(17325))*xx + T(135135);
	T b = ((T(28)*xx + T(3150))*xx + T(62370))*xx + T(135135);
	return x*a/b;
}

TEMPLATE INNER T
process_stage(stage *s, freqdata fd, T in)
{
	T temp = (in + s->sumback)*VT2*fd.L_p0*fd.g;
	T out = temp + s->sum;
	s->sum += T(2)*temp;
	s->dout = tanh2<T>(out/VT2);
	s->sumback = in*fd.L_r1 - s->dout*fd.L_q0;
	return out;
}

TEMPLATE INNER T
process_one(T in, personal *data)
{
	freqdata fd = data->fd;

	in *= data->drive;

	T sum = in + data->sumback1;
	T pre = -fd.p0*sum;
	        process_stage<T>(&data->s1, fd, tanh2<T>(pre/VT2));
	        process_stage<T>(&data->s2, fd, data->s1.dout);
	        process_stage<T>(&data->s3, fd, data->s2.dout);
	T out = process_stage<T>(&data->s4, fd, data->s3.dout);

	T back = data->feedback*out;
	data->sumback1 = fd.r1*in + fd.q0*back + data->sumback2;
	data->sumback2 = fd.r2*in + fd.q1*back + data->sumback3;
	data->sumback3 = fd.r3*in + fd.q2*back + data->sumback4;
	data->sumback4 = fd.r4*in + fd.q3*back;

	T compensate = -(data->feedback + T(1));
	return out/data->drive*compensate;
}

template<typename T>
static void
process(personal *data,
    T *in_L, T *in_R,
    T *out_L, T *out_R,
    ulong count)
{
	disable_denormals();
	v2df buf[BLOCK_SIZE];
	v2df over[FULL_SIZE];

	halfband_t<v2df> *hb_up   = &data->hb_up;
	halfband_t<v2df> *hb_down = &data->hb_down;

	for (ulong pos = 0; pos < count; pos += BLOCK_SIZE) {
		ulong rem = BLOCK_SIZE;
		if (pos + BLOCK_SIZE > count)
			rem = count - pos;

		ulong rem2 = rem*OVERSAMPLING;

		for (ulong i = 0; i < rem; i++) {
			buf[i][0] = in_L[i];
			buf[i][1] = in_R[i];
		}

		for (ulong i = 0; i < rem; i++) {
			over[i*2+0] = interpolate_a(hb_up, buf[i]);
			over[i*2+1] = interpolate_b(hb_up, buf[i]);
		}

		for (ulong i = 0; i < rem2; i++) {
			over[i] = process_one(over[i], data);
		}

		for (ulong i = 0; i < rem; i++) {
			         decimate_a(hb_down, over[i*2+0]);
			buf[i] = decimate_b(hb_down, over[i*2+1]);
		}

		for (ulong i = 0; i < rem; i++) {
			out_L[i] = buf[i][0];
			out_R[i] = buf[i][1];
		}

		in_L += BLOCK_SIZE;
		in_R += BLOCK_SIZE;
		out_L += BLOCK_SIZE;
		out_R += BLOCK_SIZE;
	}
}

INNER void
construct(personal *data)
{
	memset(data, 0, sizeof(personal));
}

INNER void
construct_params(param *params)
{
	sprintf(params[0].name, "Frequency");
	params[0].min = 20;
	params[0].max = 20000;
	params[0].scale = SCALE_HZ;
	params[0].def = DEFAULT_MAX;

	sprintf(params[1].name, "Drive");
	params[1].min = -40;
	params[1].max = 0;
	params[1].scale = SCALE_DB;
	params[1].def = DEFAULT_MIN;

	sprintf(params[2].name, "Feedback");
	params[2].min = 0;
	params[2].max = 1;
	params[2].scale = SCALE_FLOAT;
	params[2].def = DEFAULT_MIN;

	param_reset(&params[0]);
	param_reset(&params[1]);
	param_reset(&params[2]);
}

INNER void
destruct(personal *data)
{}

INNER void
resume(personal *data)
{
	memset(&data->hb_up,   0, sizeof(halfband_t<v2df>));
	memset(&data->hb_down, 0, sizeof(halfband_t<v2df>));
}

INNER void
pause(personal *data)
{}

INNER void
adjust(personal *data, param *params, ulong fs_long)
{
	double fs = fs_long;
	data->fs = fs_long;
	double f = params[0].value;
	if (f < 20) f = 20;
	if (f > fs/6*OVERSAMPLING) f = fs/6*OVERSAMPLING;
	double drive = DB2LIN(params[1].value);
	double k = params[2].value*N;
	data->drive = (v2df){drive, drive};
	data->feedback = (v2df){k, k};

	double bc1 = -4; //-binomial(N, 1);
	double bc2 = -6; //-binomial(N, 2);
	double bc3 = -4; //-binomial(N, 3);
	double bc4 = -1; //-binomial(N, 4);

	freqdata fd;
	#define fd_set(L, R) double L = R; fd.L = (v2df){L, L}
	fd_set(g, tan(M_PI*f/fs/OVERSAMPLING));
	double gg1 = g/(g + 1);
	double gg1Nk = k*gg1*gg1*gg1*gg1;
	double g1g1 = (g - 1)/(g + 1);

	fd_set(p0, 1/(1 + gg1Nk));
	fd_set(r1, bc1*gg1Nk);
	fd_set(r2, bc2*gg1Nk);
	fd_set(r3, bc3*gg1Nk);
	fd_set(r4, bc4*gg1Nk);
	fd_set(q0, r1 + bc1*g1g1);
	fd_set(q1, r2 + bc2*g1g1*g1g1);
	fd_set(q2, r3 + bc3*g1g1*g1g1*g1g1);
	fd_set(q3, r4 + bc4*g1g1*g1g1*g1g1*g1g1);

	fd_set(L_p0, 1/(1 + g));
	fd_set(L_q0, 1 - g);
	fd_set(L_r1, -g);
	#undef fd_set

	data->fd = fd;
}

INNER void
adjust_one(personal *data, param *params, unsigned int index)
{
	adjust(data, params, data->fs);
}
