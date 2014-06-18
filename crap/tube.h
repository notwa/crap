#include <string.h>
#include <stdio.h>

#include "util.h"
#include "param.h"
#include "os6iir.h"

#define ID 0x50F7BA11
#define LABEL "crap_tube"
#define NAME "crap Tube Distortion"
#define AUTHOR "Connor Olding"
#define COPYRIGHT "MIT"
#define PARAMETERS 2

#define OVERSAMPLING 6
#define HIST_SIZE_2 (2 + 2*8)
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
	oversample(h1, process_one(OVERSAMPLING*oversample(h0, SAMP), \
	    smooth(&data->drive), smooth(&data->wet)))
	    doit(x);
	    doit(0);
	    doit(0);
	    doit(0);
	    doit(0);
	y = doit(0);
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
	data->drive.speed = 1 + 20/fs/OVERSAMPLING;
	data->wet.speed = 20/fs/OVERSAMPLING;
	data->drive.log = 1;
	data->wet.log = 0;
}

static void
adjust_one(personal *data, param *params, unsigned int index)
{
	data->drive.desired = DB2LIN(params[0].value);
	data->wet.desired = params[1].value;
}
