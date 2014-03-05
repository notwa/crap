#include <string.h>

#include "util.h"

#define ID 0x0DEFACED
#define LABEL "crap_eq_const"
#define NAME "crap Constant Equalizer"
#define AUTHOR "Connor Olding"
#define COPYRIGHT "MIT"
#define PARAMETERS 0

#define BANDS 6
typedef struct {
	biquad filters[2][BANDS];
} personal;

static double
process_one(biquad *filters, double samp)
{
	for (int i = 0; i < BANDS; i++)
		samp = biquad_run(&filters[i], samp);
	return samp;
}

static void
process(personal *data,
    float *in_L, float *in_R,
    float *out_L, float *out_R,
    unsigned long count)
{
	disable_denormals();
	for (unsigned long pos = 0; pos < count; pos++) {
		out_L[pos] = process_one(data->filters[0], in_L[pos]);
		out_R[pos] = process_one(data->filters[1], in_R[pos]);
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
		out_L[pos] = process_one(data->filters[0], in_L[pos]);
		out_R[pos] = process_one(data->filters[1], in_R[pos]);
	}
}

static void
construct(personal *data)
{}

static void
destruct(personal *data)
{}

static void
resume(personal *data)
{
	biquad *filters = data->filters[0];
	for (int i = 0; i < BANDS; i++)
		biquad_init(&filters[i]);
	memcpy(data->filters[1], filters, BANDS*sizeof(biquad));
}

static void
pause(personal *data)
{}

static void
adjust(personal *data, unsigned long fs)
{
	biquad *filters = data->filters[0];
	filters[0] = biquad_gen(FILT_PEAKING,   37.01, +3.3, 1.94, fs);
	filters[1] = biquad_gen(FILT_PEAKING,   2208., -4.0, 0.56, fs);
	filters[2] = biquad_gen(FILT_PEAKING,   5236., +2.3, 0.81, fs);
	filters[3] = biquad_gen(FILT_PEAKING,   8092., -3.5, 0.43, fs);
	filters[4] = biquad_gen(FILT_PEAKING,   90.88, -1.0, 0.87, fs);
	filters[5] = biquad_gen(FILT_HIGHPASS,  16.00,    0, 1.00, fs);
}
