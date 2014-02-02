#include <stdio.h>
#include <string.h>

#define BIQUAD_DOUBLE
#include "crap_util.h"
#include "param.h"

#define BANDS 4

#define ID 0x000CAFED
#define LABEL "crap_eq"
#define NAME "crap Parametric Equalizer"
#define AUTHOR "Connor Olding"
#define COPYRIGHT "MIT"
#define PARAMETERS (BANDS*3)

typedef struct {
	biquad filters[2][BANDS];
	float fs;
} personal;

static bq_t
process_one(biquad *filters, bq_t samp)
{
	for (int i = 0; i < BANDS; i++)
		samp = biquad_run(&filters[i], samp);
	return samp;
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
	for (unsigned long pos = 0; pos < count; pos++) {
		out_L[pos] = process_one(data->filters[0], in_L[pos]);
		out_R[pos] = process_one(data->filters[1], in_R[pos]);
	}
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
construct_params(param *params) {
	for (int i = 0; i < BANDS; i++) {
		sprintf(params[0].name, "Band %i Frequency", i + 1);
		params[0].min = 20;
		params[0].max = 20000;
		params[0].scale = SCALE_HZ;
		params[0].def = DEFAULT_440;
		param_reset(&params[0]);

		sprintf(params[1].name, "Band %i Gain", i + 1);
		params[1].min = -18;
		params[1].max = 18;
		params[1].scale = SCALE_DB;
		params[1].def = DEFAULT_0;
		param_reset(&params[1]);

		sprintf(params[2].name, "Band %i Bandwidth", i + 1);
		params[2].min = 0.0625;
		params[2].max = 8;
		params[2].scale = SCALE_FLOAT;
		params[2].def = DEFAULT_1;
		param_reset(&params[2]);

		params += 3;
	}
}

static void
construct(personal *data) {
}

static void
destruct(personal *data) {
}

static void
adjust(personal *data, param *params, unsigned long fs) {
	data->fs = fs;
	biquad *filters = data->filters[0];
	for (int i = 0; i < BANDS; i++) {
		filters[i] = biquad_gen(0,
		    params[0].value, params[1].value, params[2].value, fs);
		params += 3;
	}
	resume(data);
}

static void
adjust_one(personal *data, param *params, unsigned int index) {
	float fs = data->fs;
	params += index/3*3;
	data->filters[0][index/3] = biquad_gen(0,
	    params[0].value, params[1].value, params[2].value, fs);
	resume(data);
}
