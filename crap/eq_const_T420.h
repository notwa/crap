#include <string.h>

#define ID (0x0DEFACED+420)
#define LABEL "crap_eq_const_T420"
#define NAME "crap T420 Speaker Compensation"
#define AUTHOR "Connor Olding"
#define COPYRIGHT "MIT"
#define PARAMETERS 0

#define BLOCK_SIZE 256

#include "util.h"

#define BANDS 12
typedef struct {
	biquad filters[2][BANDS];
} personal;

static void
process_double(personal *data,
    double *in_L, double *in_R,
    double *out_L, double *out_R,
    unsigned long count)
#include "process_biquads.h"

static void
process(personal *data,
    float *in_L, float *in_R,
    float *out_L, float *out_R,
    ulong count)
#include "process_biquads.h"

INNER void
construct(personal *data)
{}

INNER void
destruct(personal *data)
{}

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
adjust(personal *data, ulong fs)
{
	biquad *filters = data->filters[0];
	// TODO: reduce ~3dB peaks at 1680, 2200
	// TODO: reduce small peak at 600, 3780, 10600
	filters[ 0] = biquad_gen(FILT_PEAKING,    150.,  12., 1.50, fs);
	filters[ 1] = biquad_gen(FILT_PEAKING,    640.,  7.0, 0.80, fs);
	filters[ 2] = biquad_gen(FILT_PEAKING,    1220, -12., 0.70, fs);
	filters[ 3] = biquad_gen(FILT_PEAKING,    1670,  9.0, 0.18, fs);
	filters[ 4] = biquad_gen(FILT_PEAKING,    2270,  9.0, 0.20, fs);
	filters[ 5] = biquad_gen(FILT_PEAKING,    2470, -7.0, 0.16, fs);
	filters[ 6] = biquad_gen(FILT_PEAKING,    3200, -0.1, 0.25, fs);
	filters[ 7] = biquad_gen(FILT_PEAKING,    6200, -3.5, 0.25, fs);
	filters[ 8] = biquad_gen(FILT_PEAKING,    6000, -9.0, 4.00, fs);
	filters[ 9] = biquad_gen(FILT_PEAKING,   11560,  6.0, 0.12, fs);
	filters[10] = biquad_gen(FILT_HIGHSHELF, 11000,  5.0, 0.50, fs);
	filters[11] = biquad_gen(FILT_HIGHPASS,    120,  0.0, 1.00, fs);
}
