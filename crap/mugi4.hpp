#define BLOCK_SIZE 256
#define OVERSAMPLING 2
#define FULL_SIZE (BLOCK_SIZE*OVERSAMPLING)

/*
an implementation of:
S. D Angelo and V. Välimäki. Generalized Moog Ladder Filter: Part II
Explicit Nonlinear Model through a Novel Delay-Free Loop Implementation Method.
IEEE Trans. Audio, Speech, and Lang. Process.,
vol. 22, no. 12, pp. 1873 1883, December 2014.
https://aaltodoc.aalto.fi/bitstream/handle/123456789/14420/article6.pdf
*/

#include <stdio.h>
#include <string.h>

#include "util.hpp"
#include "Param.hpp"
#include "Crap.hpp"
#include "os2piir.hpp"
#include "Buffer2OS2.hpp"

#define VT 0.026
#define N 4
#define VT2 T(2.*VT)

TEMPLATE INNER PURE T
tanh2(T x)
{
	//return T(tanh(x[0]), tanh(x[1]));
	T xx = x*x;
	T a = ((xx + T(378))*xx + T(17325))*xx + T(135135);
	T b = ((T(28)*xx + T(3150))*xx + T(62370))*xx + T(135135);
	return x*a/b;
}

struct freqdata {
	v2df g;
	v2df p0;
	v2df q0, q1, q2, q3;
	v2df r1, r2, r3, r4;
	v2df L_p0;
	v2df L_q0;
	v2df L_r1;
};

struct stage {
	v2df sum, sumback, dout;

	TEMPLATE inline T
	process(freqdata fd, T in)
	{
		T temp = (in + sumback)*VT2*fd.L_p0*fd.g;
		T out = temp + sum;
		sum += T(2)*temp;
		dout = tanh2<T>(out/VT2);
		sumback = in*fd.L_r1 - dout*fd.L_q0;
		return out;
	}
};

struct mugi4 {
	freqdata fd;
	stage s1, s2, s3, s4;
	v2df sumback1, sumback2, sumback3, sumback4;
	v2df drive, feedback;

	TEMPLATE inline T
	process(T in)
	{
		in *= drive;

		T sum = in + sumback1;
		T pre = -fd.p0*sum;
			s1.process<T>(fd, tanh2<T>(pre/VT2));
			s2.process<T>(fd, s1.dout);
			s3.process<T>(fd, s2.dout);
		T out = s4.process<T>(fd, s3.dout);

		T back = feedback*out;
		sumback1 = fd.r1*in + fd.q0*back + sumback2;
		sumback2 = fd.r2*in + fd.q1*back + sumback3;
		sumback3 = fd.r3*in + fd.q2*back + sumback4;
		sumback4 = fd.r4*in + fd.q3*back;

		T compensate = -(feedback + T(1));
		return out/drive*compensate;
	}

	inline void
	setup(double wc, double drive_, double feedback_)
	{
		drive = v2df(drive_);
		feedback = v2df(feedback_);
		double k = feedback_;

		double bc1 = -4; //-binomial(N, 1);
		double bc2 = -6; //-binomial(N, 2);
		double bc3 = -4; //-binomial(N, 3);
		double bc4 = -1; //-binomial(N, 4);

		// apparently fd_set is used by some stdio implementations, so
		#define crap_fd_set(L, R) double L = R; fd.L = (v2df){L, L}
		crap_fd_set(g, tan(wc));
		double gg1 = g/(g + 1);
		double gg1Nk = k*gg1*gg1*gg1*gg1;
		double g1g1 = (g - 1)/(g + 1);

		crap_fd_set(p0, 1/(1 + gg1Nk));
		crap_fd_set(r1, bc1*gg1Nk);
		crap_fd_set(r2, bc2*gg1Nk);
		crap_fd_set(r3, bc3*gg1Nk);
		crap_fd_set(r4, bc4*gg1Nk);
		crap_fd_set(q0, r1 + bc1*g1g1);
		crap_fd_set(q1, r2 + bc2*g1g1*g1g1);
		crap_fd_set(q2, r3 + bc3*g1g1*g1g1*g1g1);
		crap_fd_set(q3, r4 + bc4*g1g1*g1g1*g1g1*g1g1);

		crap_fd_set(L_p0, 1/(1 + g));
		crap_fd_set(L_q0, 1 - g);
		crap_fd_set(L_r1, -g);
		#undef crap_fd_set
	}
};

struct Crap_mugi4
:public AdjustAll<Buffer2OS2<Crap>> {
	static constexpr ulong id = 0xD8D0D8D0;
	static constexpr char label[] = "crap_mugi4";
	static constexpr char name[] = "crap mugi4 (moog-like)";
	static constexpr char author[] = "Connor Olding";
	static constexpr char copyright[] = "MIT";

	static constexpr ulong parameters = 3;

	mugi4 filter;

	inline
	Crap_mugi4()
	{
		memset(&filter, 0, sizeof(mugi4));
	}

	inline void
	process2(v2df *buf, ulong rem)
	{
		for (ulong i = 0; i < rem; i++)
			buf[i] = filter.process(buf[i]);
	}

	static inline void
	construct_params(Param *params)
	{
		sprintf(params[0].name, "Frequency");
		params[0].min = 20;
		params[0].max = 20000;
		params[0].scale = SCALE_HZ;
		params[0].def = DEFAULT_MAX;
		params[0].reset();

		sprintf(params[1].name, "Drive");
		params[1].min = -40;
		params[1].max = 0;
		params[1].scale = SCALE_DB;
		params[1].def = DEFAULT_MIN;
		params[1].reset();

		sprintf(params[2].name, "Feedback");
		params[2].min = 0;
		params[2].max = 1;
		params[2].scale = SCALE_FLOAT;
		params[2].def = DEFAULT_MIN;
		params[2].reset();
	}

	inline void
	pause()
	{}

	inline void
	resume()
	{
		memset(&hb_up,   0, sizeof(halfband_t<v2df>));
		memset(&hb_down, 0, sizeof(halfband_t<v2df>));
	}

	inline void
	adjust_all(Param *params)
	{
		double f = params[0].value;
		if (f < 20) f = 20;
		if (f > fs/6*OVERSAMPLING) f = fs/6*OVERSAMPLING;
		double wc = M_PI*f/fs/OVERSAMPLING;
		double drive = DB2LIN(params[1].value);
		double feedback = params[2].value*N;
		filter.setup(wc, drive, feedback);
	}
};

constexpr char Crap_mugi4::label[];
constexpr char Crap_mugi4::name[];
constexpr char Crap_mugi4::author[];
constexpr char Crap_mugi4::copyright[];
