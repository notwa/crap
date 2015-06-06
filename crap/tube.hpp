#include <string.h>
#include <stdio.h>

#define ID 0x50F7BA11
#define LABEL "crap_tube"
#define NAME "crap Tube Distortion"
#define AUTHOR "Connor Olding"
#define COPYRIGHT "MIT"
#define PARAMETERS 2

#define OVERSAMPLING 2
#define BLOCK_SIZE 256
#define FULL_SIZE (BLOCK_SIZE*OVERSAMPLING)

#include "util.hpp"
#include "param.hpp"
#include "os2piir_stereo.hpp"

typedef struct {
	double desired, actual, speed;
	int log; // use multiplication instead of addition for speed
} smoothval;

typedef struct {
	halfband_t hb_up, hb_down;
	smoothval drive, wet;
} personal;

INNER double
smooth(smoothval *val)
{
	double a = val->actual;
	double d = val->desired;
	double s = val->speed;
	double l = val->log;
	if (a < d) {
		if (l) a *= s;
		else a += s;
		if (a > d) a = d;
	} else if (a > d) {
		if (l) a /= s;
		else a -= s;
		if (a < d) a = d;
	}
	val->actual = a;
	return a;
}

INNER CONST v2df
distort(v2df x)
{
	return (V(27.)*x + V(9.)) / (V(9.)*x*x + V(6.)*x + V(19.)) - V(9./19.);
}

INNER CONST v2df
process_one(v2df x, v2df drive, v2df wet)
{
	return (distort(x*drive)/drive*V(0.79) - x)*wet + x;
}

template<typename T>
static void
process(personal *data,
    T *in_L, T *in_R,
    T *out_L, T *out_R,
    ulong count)
{
	disable_denormals();

	v2df drives[FULL_SIZE], wets[FULL_SIZE];
	v2df buf[BLOCK_SIZE];
	v2df over[FULL_SIZE];

	halfband_t *hb_up   = &data->hb_up;
	halfband_t *hb_down = &data->hb_down;

	for (ulong pos = 0; pos < count; pos += BLOCK_SIZE) {
		ulong rem = BLOCK_SIZE;
		if (pos + BLOCK_SIZE > count)
			rem = count - pos;

		ulong rem2 = rem*OVERSAMPLING;

		for (ulong i = 0; i < rem2; i++) {
			double y = smooth(&data->drive);
			drives[i] = V(y);
		}
		for (ulong i = 0; i < rem2; i++) {
			double y = smooth(&data->wet);
			wets[i] = V(y);
		}

		for (ulong i = 0; i < rem; i++) {
			buf[i][0] = in_L[i];
			buf[i][1] = in_R[i];
		}

		for (ulong i = 0; i < rem; i++) {
			over[i*2+0] = interpolate_a(hb_up, buf[i]);
			over[i*2+1] = interpolate_b(hb_up, buf[i]);
		}

		for (ulong i = 0; i < rem2; i++) {
			over[i] = process_one(over[i], drives[i], wets[i]);
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
resume(personal *data)
{
	memset(&data->hb_up,   0, sizeof(halfband_t));
	memset(&data->hb_down, 0, sizeof(halfband_t));
}

INNER void
pause(personal *data)
{}

INNER void
construct(personal *data)
{
	memset(data, 0, sizeof(personal));
}

INNER void
construct_params(param *params)
{
	sprintf(params[0].name, "Drive");
	params[0].min = -30;
	params[0].max = 15;
	params[0].scale = SCALE_DB;
	params[0].def = DEFAULT_0;

	sprintf(params[1].name, "Wetness");
	params[1].min = 0;
	params[1].max = 1;
	params[1].scale = SCALE_FLOAT;
	params[1].def = DEFAULT_1;

	param_reset(&params[0]);
	param_reset(&params[1]);
}

INNER void
destruct(personal *data)
{}

INNER void
adjust(personal *data, param *params, ulong fs_long)
{
	resume(data);
	double fs = fs_long;
	data->drive.desired = DB2LIN(params[0].value);
	data->wet.desired = params[1].value;
	data->drive.actual = data->drive.desired;
	data->wet.actual = data->wet.desired;
	data->drive.speed = 1 + 20/fs/OVERSAMPLING;
	data->wet.speed = 20/fs/OVERSAMPLING;
	data->drive.log = 1;
	data->wet.log = 0;
}

INNER void
adjust_one(personal *data, param *params, unsigned int index)
{
	data->drive.desired = DB2LIN(params[0].value);
	data->wet.desired = params[1].value;
}
