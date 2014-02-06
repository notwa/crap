#include <string.h>

#ifdef __SSE2__
#include <xmmintrin.h>
#ifndef __SSE2_MATH__
#warning SSE2 enabled but not forced, beware denormals
#endif
#else
#warning built without SSE2, denormals will be painful
#endif

#define ID 0x50F7BA11
#define LABEL "crap_tube"
#define NAME "crap Tube Distortion"
#define AUTHOR "Connor Olding"
#define COPYRIGHT "MIT"
#define PARAMETERS 0

#define HIST_SIZE   36
#define HIST_SIZE_2 18

typedef struct {
	double history_L[HIST_SIZE];
	double history_R[HIST_SIZE];
} personal;

static void
disable_denormals()
{
	#if __SSE2__
        _mm_setcsr(_mm_getcsr() | 0x8040);
	#endif
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
process_one(double h[HIST_SIZE], double x)
{
	double y;
	y = downsample(h + HIST_SIZE_2, distort(4*upsample(h, x)));
	    downsample(h + HIST_SIZE_2, distort(4*upsample(h, 0)));
	    downsample(h + HIST_SIZE_2, distort(4*upsample(h, 0)));
	    downsample(h + HIST_SIZE_2, distort(4*upsample(h, 0)));
	return y*0.71;
}

static void
process(personal *data,
    float *in_L, float *in_R,
    float *out_L, float *out_R,
    unsigned long count) {
	disable_denormals();
	for (unsigned long pos = 0; pos < count; pos++) {
		out_L[pos] = process_one(data->history_L, in_L[pos]);
		out_R[pos] = process_one(data->history_R, in_R[pos]);
	}
}

static void
process_double(personal *data,
    double *in_L, double *in_R,
    double *out_L, double *out_R,
    unsigned long count) {
	disable_denormals();
	for (unsigned long pos = 0; pos < count; pos++) {
		out_L[pos] = process_one(data->history_L, in_L[pos]);
		out_R[pos] = process_one(data->history_R, in_R[pos]);
	}
}

static void
resume(personal *data) {
	memset(data->history_L, 0, HIST_SIZE*sizeof(double));
	memset(data->history_R, 0, HIST_SIZE*sizeof(double));
}

static void
pause(personal *data) {
}
static void
construct(personal *data) {
	resume(data);
}
static void
destruct(personal *data) {
}

static void
adjust(personal *data, unsigned long fs) {
	resume(data);
}

