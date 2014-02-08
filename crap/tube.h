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

#define HIST_SIZE   36
#define HIST_SIZE_2 18

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
upsample(double h[HIST_SIZE_2], double x)
{
	double y;
	// -3 at 18600/44100/2, -96 stopband
	// designed with <https://coewww.rutgers.edu/~orfanidi/hpeq/>
	LOWPASS(0, +0.71327159,+0.00688573,-0.45391337,+0.88734229);
	LOWPASS(1, +0.63347126,+0.05572752,-0.36946634,+0.69213639);
	LOWPASS(2, +0.55963645,+0.13990391,-0.26487901,+0.52405582);
	LOWPASS(3, +0.49037095,+0.24706928,-0.14763065,+0.37544183);
	LOWPASS(4, +0.42692239,+0.36379839,-0.02763286,+0.24527604);
	LOWPASS(5, +0.37268890,+0.47433865,+0.08224090,+0.13747554);
	LOWPASS(6, +0.33241251,+0.56148939,+0.16727062,+0.05904378);
	LOWPASS(7, +0.31079382,+0.60975767,+0.21392163,+0.01742368);
	BQSHIFT(8);
	return y;
}

static double
downsample(double h[HIST_SIZE_2], double x)
{
	double y;
	// -3 at 17544/44100/4, -96 stopband
	// same idea as upsample()
	LOWPASS(0, +0.62136966,-0.87573986,-1.56336581,+0.93036527);
	LOWPASS(1, +0.56540370,-0.77393348,-1.44258778,+0.79946170);
	LOWPASS(2, +0.49824084,-0.63630306,-1.31114921,+0.67132784);
	LOWPASS(3, +0.41949184,-0.46466704,-1.16600279,+0.54031944);
	LOWPASS(4, +0.33172375,-0.26684785,-1.00993399,+0.40653364);
	LOWPASS(5, +0.24269774,-0.06242297,-0.85492245,+0.27789496);
	LOWPASS(6, +0.16673206,+0.11379847,-0.72421195,+0.17147454);
	LOWPASS(7, +0.12199271,+0.21811002,-0.64769184,+0.10978728);
	BQSHIFT(8);
	return y;
}

static double
process_one(double x, double drive, double wet)
{
	return (distort(x*drive)/drive*0.79 - x)*wet + x;
}

static double
oversample(personal *data, double x, int right)
{
	double *h0 = (!right) ? data->history_L : data->history_R;
	double *h1 = h0 + HIST_SIZE_2;
	double y;

	#define doit(SAMP) \
	downsample(h1, process_one(4*upsample(h0, SAMP), \
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
		out_L[pos] = oversample(data, in_L[pos], 0);
		out_R[pos] = oversample(data, in_R[pos], 1);
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
		out_L[pos] = oversample(data, in_L[pos], 0);
		out_R[pos] = oversample(data, in_R[pos], 1);
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
