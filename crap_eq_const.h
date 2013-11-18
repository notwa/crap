#include <string.h>

#define BIQUAD_DOUBLE
#include "crap_util.h"

#define ID 0x0DEFACED
#define LABEL "crap_eq_const"
#define NAME "crap Constant Equalizer"
#define AUTHOR "Connor Olding"
#define COPYRIGHT "MIT"
#define PARAMETERS 0

#define BANDS 4
typedef struct {
	biquad filters[2][BANDS];
} personal;

static bq_t
process_one(biquad *filters, bq_t samp)
{
	for (int i = 0; i < BANDS; i++)
		samp = biquad_run(&filters[i], samp);
	return samp*1.2023;
}

static void
process(personal *data,
    float *in_L, float *in_R,
    float *out_L, float *out_R,
    unsigned long count) {
	for (unsigned long pos = 0; pos < count; pos++) {
		out_L[pos] = process_one(data->filters[0], in_L[pos]);
		out_R[pos] = process_one(data->filters[1], in_R[pos]);
	}
}

static void
process_double(personal *data,
    double *in_L, double *in_R,
    double *out_L, double *out_R,
    unsigned long count) {
	// TODO: test which hosts use this
	for (unsigned long pos = 0; pos < count; pos++) {
		out_L[pos] = process_one(data->filters[0], in_L[pos]);
		out_R[pos] = process_one(data->filters[1], in_R[pos]);
	}
}

static void
construct(personal *data) {
}

static void
destruct(personal *data) {
}

static void
resume(personal *data) {
	biquad *filters = data->filters[0];
	for (int i = 0; i < BANDS; i++)
		biquad_init(&filters[i]);
	memcpy(data->filters[1], filters, BANDS*sizeof(biquad));
}

static void
pause(personal *data) {
}

static void
adjust(personal *data, unsigned long fs) {
	biquad *filters = data->filters[0];
	filters[0] = biquad_gen(0, 34.34, +4.6, 1.21, fs);
	filters[1] = biquad_gen(0, 85.74, -1.2, 1.31, fs);
	filters[2] = biquad_gen(2, 862.2, -5.5, 1.00, fs);
	filters[3] = biquad_gen(0, 7496., +3.3, 1.10, fs);
}
