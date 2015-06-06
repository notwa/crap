#include <stdio.h>
#include <string.h>

#define BANDS 4

#define ID 0x000CAFED
#define LABEL "crap_eq"
#define NAME "crap Parametric Equalizer"
#define AUTHOR "Connor Olding"
#define COPYRIGHT "MIT"
#define PARAMETERS (BANDS*3)

#define BLOCK_SIZE 256

#include "util.hpp"
#include "param.hpp"

typedef struct {
	biquad filters[2][BANDS];
	float fs;
} personal;

template<typename T>
static void
process(personal *data,
    T *in_L, T *in_R,
    T *out_L, T *out_R,
    unsigned long count)
{
	disable_denormals();

	v2df buf[BLOCK_SIZE];

	biquad *f0, *f1;

	for (ulong pos = 0; pos < count; pos += BLOCK_SIZE) {
		ulong rem = BLOCK_SIZE;
		if (pos + BLOCK_SIZE > count)
			rem = count - pos;

		for (ulong i = 0; i < rem; i++) {
			buf[i][0] = in_L[i];
			buf[i][1] = in_R[i];
		}

		f0 = data->filters[0];
		f1 = data->filters[1];
		for (ulong i = 0; i < BANDS; i++) {
			biquad_run_block_stereo(f0, f1, buf, rem);
			f0++;
			f1++;
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
	biquad *filters = data->filters[0];
	for (int i = 0; i < BANDS; i++)
		biquad_init(&filters[i]);
	memcpy(data->filters[1], filters, BANDS*sizeof(biquad));
}

INNER void
pause(personal *data)
{}

INNER void
construct_params(param *params)
{
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
		params[2].min = 0.0625; // 2^-4 could probably be 2^-3
		params[2].max = 8;      // 2^3  could probably be 2^2
		params[2].scale = SCALE_FLOAT;
		params[2].def = DEFAULT_1;
		param_reset(&params[2]);

		params += 3;
	}
}

INNER void
construct(personal *data)
{}

INNER void
destruct(personal *data)
{}

INNER void
adjust(personal *data, param *params, unsigned long fs)
{
	data->fs = fs;
	biquad *filters = data->filters[0];
	for (int i = 0; i < BANDS; i++) {
		filters[i] = biquad_gen(FILT_PEAKING,
		    params[0].value, params[1].value, params[2].value, fs);
		params += 3;
	}
	resume(data);
}

INNER void
adjust_one(personal *data, param *params, unsigned int index)
{
	float fs = data->fs;
	params += index/3*3;
	data->filters[0][index/3] = biquad_gen(FILT_PEAKING,
	    params[0].value, params[1].value, params[2].value, fs);
	resume(data);
}
