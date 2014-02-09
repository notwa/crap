#include <string.h>
#include <stdio.h>

#include "util.h"
#include "param.h"

#define ID 0x50F7BA11
#define LABEL "crap_tube"
#define NAME "crap Tube Distortion"
#define AUTHOR "Connor Olding"
#define COPYRIGHT "MIT"
#define PARAMETERS 2

#define HIST_SIZE_2 (2 + 2*10)
#define HIST_SIZE   (HIST_SIZE_2*2)

typedef struct {
	double desired, actual, speed;
	int log; // use multiplication instead of addition for speed
} smoothval;

typedef struct {
	double history_L[HIST_SIZE];
	double history_R[HIST_SIZE];
	smoothval drive, wet;
} personal;

static double
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

static double
distort(double x)
{
	return (27*x + 9) / (9*x*x + 6*x + 19) - 9/19.;
}

#define BQSHIFT(i) \
	h[i*2 + 1] = h[i*2 + 0]; \
	h[i*2 + 0] = x;

// b2 is always b0 with lowpasses
// a0 is already factored into the rest of the coefficients
#define LOWPASS(i, b0, b1, a1, a2) \
	y = b0*x + b1*h[i*2 + 0] + b0*h[i*2 + 1] \
	         - a1*h[i*2 + 2] - a2*h[i*2 + 3]; \
	BQSHIFT(i); \
	x = y;

static double
oversample(double h[HIST_SIZE_2], double x)
{
	/* designed with <https://coewww.rutgers.edu/~orfanidi/hpeq/>
	fs = 44100; OS = 4; Gs = -120; f2 = 18248/fs/OS; f2s = 22050/fs/OS;
	N = hpeqord(-Inf,0,Gs,-6,2*pi*f2s,2*pi*f2,2) % 20.9971490662433
	*/
	double y;
	LOWPASS(0, +0.65855672,-0.92949215,-1.56012668,+0.94774797);
	LOWPASS(1, +0.61716958,-0.85685555,-1.47099672,+0.84848033);
	LOWPASS(2, +0.56959704,-0.76263107,-1.37527450,+0.75183750);
	LOWPASS(3, +0.51503506,-0.64652564,-1.27052746,+0.65407194);
	LOWPASS(4, +0.45328074,-0.50918096,-1.15569311,+0.55307363);
	LOWPASS(5, +0.38524858,-0.35361043,-1.03185791,+0.44874463);
	LOWPASS(6, +0.31367929,-0.18703461,-0.90341296,+0.34373693);
	LOWPASS(7, +0.24382090,-0.02260789,-0.77918770,+0.24422161);
	LOWPASS(8, +0.18353302,+0.12028336,-0.67260228,+0.15995168);
	LOWPASS(9, +0.14205849,+0.21898074,-0.59952639,+0.10262410);
	BQSHIFT(10);
	return y;
}

static double
process_one(double x, double drive, double wet)
{
	return (distort(x*drive)/drive*0.79 - x)*wet + x;
}

static double
process_os(personal *data, double x, int right)
{
	double *h0 = (!right) ? data->history_L : data->history_R;
	double *h1 = h0 + HIST_SIZE_2;
	double y;

	#define doit(SAMP) \
	oversample(h1, process_one(4*oversample(h0, SAMP), \
	    smooth(&data->drive), smooth(&data->wet)))
	y = doit(x);
	    doit(0);
	    doit(0);
	    doit(0);
	#undef doit

	return y;
}

static void
process(personal *data,
    float *in_L, float *in_R,
    float *out_L, float *out_R,
    unsigned long count)
{
	disable_denormals();
	for (unsigned long pos = 0; pos < count; pos++) {
		out_L[pos] = process_os(data, in_L[pos], 0);
		out_R[pos] = process_os(data, in_R[pos], 1);
	}
}

static void
process_double(personal *data,
    double *in_L, double *in_R,
    double *out_L, double *out_R,
    unsigned long count)
{
	disable_denormals();
	for (unsigned long pos = 0; pos < count; pos++) {
		out_L[pos] = process_os(data, in_L[pos], 0);
		out_R[pos] = process_os(data, in_R[pos], 1);
	}
}

static void
resume(personal *data)
{
	memset(data->history_L, 0, HIST_SIZE*sizeof(double));
	memset(data->history_R, 0, HIST_SIZE*sizeof(double));
}

static void
pause(personal *data)
{}

static void
construct(personal *data)
{
	memset(data, 0, sizeof(personal));
}

static void
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

static void
destruct(personal *data)
{}

static void
adjust(personal *data, param *params, unsigned long fs_long)
{
	resume(data);
	double fs = fs_long;
	data->drive.desired = DB2LIN(params[0].value);
	data->wet.desired = params[1].value;
	data->drive.actual = data->drive.desired;
	data->wet.actual = data->wet.desired;
	data->drive.speed = 1 + 20/fs/4;
	data->wet.speed = 20/fs/4;
	data->drive.log = 1;
	data->wet.log = 0;
}

static void
adjust_one(personal *data, param *params, unsigned int index)
{
	data->drive.desired = DB2LIN(params[0].value);
	data->wet.desired = params[1].value;
}
