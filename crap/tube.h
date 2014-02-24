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

#define BQSHIFT(i) \
	h[i*2 + 1] = h[i*2 + 0]; \
	h[i*2 + 0] = x;

// b0 and b2 are equivalent and factored out
// b0 and a0 are factored out as overall gain
#define LOWPASS(i, b1, a1, a2) \
	y = x + b1*h[i*2 + 0] +    h[i*2 + 1] \
	      - a1*h[i*2 + 2] - a2*h[i*2 + 3]; \
	BQSHIFT(i); \
	x = y;

static double
oversample(double h[HIST_SIZE_2], double x)
{
	/* Fp = 20000.62
	   os = 6; N = 8*2; Gp = 0.0135; Gs = 120;
	*/
	double y;
	LOWPASS(0,-1.7310964991540,-1.7686201550064,+0.9924894080401);
	LOWPASS(1,-1.7228987922703,-1.7591070571963,+0.9756664940293);
	LOWPASS(2,-1.7030928294094,-1.7508784553758,+0.9530819894740);
	LOWPASS(3,-1.6623628189757,-1.7420481292930,+0.9206849371458);
	LOWPASS(4,-1.5767803723821,-1.7314029454409,+0.8755898544307);
	LOWPASS(5,-1.3754954625889,-1.7189289035735,+0.8193999703446);
	LOWPASS(6,-0.7934216412437,-1.7066722500921,+0.7627918423024);
	LOWPASS(7,+1.1176227011686,-1.6987049565355,+0.7256338727337);
	BQSHIFT(8);
	return y*0.000005162322938;
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
