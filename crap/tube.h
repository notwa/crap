#include <alloca.h>
#include <string.h>
#include <stdio.h>

#include "util.h"
#include "param.h"
#include "os2piir.h"

#define ID 0x50F7BA11
#define LABEL "crap_tube"
#define NAME "crap Tube Distortion"
#define AUTHOR "Connor Olding"
#define COPYRIGHT "MIT"
#define PARAMETERS 2

#define OVERSAMPLING 2
#define BLOCK_SIZE 256
#define FULL_SIZE (BLOCK_SIZE*OVERSAMPLING)

typedef unsigned long ulong;

typedef struct {
	double desired, actual, speed;
	int log; // use multiplication instead of addition for speed
} smoothval;

typedef struct {
	halfband_t hbu_L, hbu_R, hbd_L, hbd_R;
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

INNER double
distort(double x)
{
	return (27*x + 9) / (9*x*x + 6*x + 19) - 9/19.;
}

INNER double
process_one(double x, double drive, double wet)
{
	return (distort(x*drive)/drive*0.79 - x)*wet + x;
}

INNER void
process_double(personal *data,
    double *in_L, double *in_R,
    double *out_L, double *out_R,
    ulong count)
{
	disable_denormals();

	double drives[FULL_SIZE], wets[FULL_SIZE];
	double in_os[FULL_SIZE], out_os[FULL_SIZE];

	for (ulong pos = 0; pos < count; pos += BLOCK_SIZE) {
		ulong rem = BLOCK_SIZE;
		if (pos + BLOCK_SIZE > count)
			rem = count - pos;

		for (ulong i = 0; i < rem*OVERSAMPLING; i++)
			drives[i] = smooth(&data->drive);
		for (ulong i = 0; i < rem*OVERSAMPLING; i++)
			wets[i] = smooth(&data->wet);

		halfband_t *hb;

		// left channel
		hb = &data->hbu_L;
		for (ulong i = 0, j = 0; j < rem; i += OVERSAMPLING, j++) {
			in_os[i+0] = interpolate(hb, in_L[j]);
			in_os[i+1] = interpolate(hb, in_L[j]);
		}

		for (ulong i = 0; i < rem*OVERSAMPLING; i++) {
			out_os[i] = process_one(in_os[i], drives[i], wets[i]);
		}

		hb = &data->hbd_L;
		for (ulong i = 0, j = 0; j < rem; i += OVERSAMPLING, j++) {
			decimate(hb, out_os[i+0]);
			out_L[j] = decimate(hb, out_os[i+1]);
		}

		// right channel
		hb = &data->hbu_R;
		for (ulong i = 0, j = 0; j < rem; i += OVERSAMPLING, j++) {
			in_os[i+0] = interpolate(hb, in_R[j]);
			in_os[i+1] = interpolate(hb, in_R[j]);
		}

		for (ulong i = 0; i < rem*OVERSAMPLING; i++) {
			out_os[i] = process_one(in_os[i], drives[i], wets[i]);
		}

		hb = &data->hbd_R;
		for (ulong i = 0, j = 0; j < rem; i += OVERSAMPLING, j++) {
			decimate(hb, out_os[i+0]);
			out_R[j] = decimate(hb, out_os[i+1]);
		}

		in_L += BLOCK_SIZE;
		in_R += BLOCK_SIZE;
		out_L += BLOCK_SIZE;
		out_R += BLOCK_SIZE;
	}
}

#include "process.h"

INNER void
resume(personal *data)
{
	memset(&data->hbu_L, 0, sizeof(halfband_t));
	memset(&data->hbu_R, 0, sizeof(halfband_t));
	memset(&data->hbd_L, 0, sizeof(halfband_t));
	memset(&data->hbd_R, 0, sizeof(halfband_t));
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
