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
	filters[ 0] = biquad_gen(FILT_PEAKING,    180.,  11., 1.40, fs);
	filters[ 1] = biquad_gen(FILT_PEAKING,    740.,  5.5, 0.70, fs);
	filters[ 2] = biquad_gen(FILT_PEAKING,    1220, -12., 0.70, fs);
	filters[ 3] = biquad_gen(FILT_PEAKING,    1670,  6.0, 0.18, fs);
	filters[ 4] = biquad_gen(FILT_PEAKING,    2080, -2.5, 0.30, fs);
	filters[ 5] = biquad_gen(FILT_PEAKING,    2270,  6.0, 0.20, fs);
	filters[ 6] = biquad_gen(FILT_PEAKING,    2470, -7.0, 0.16, fs);
	filters[ 7] = biquad_gen(FILT_PEAKING,    3780, -1.5, 0.38, fs);
	filters[ 8] = biquad_gen(FILT_PEAKING,    6200, -3.5, 0.25, fs);
	filters[ 9] = biquad_gen(FILT_PEAKING,    6000, -9.0, 4.00, fs);
	filters[10] = biquad_gen(FILT_HIGHSHELF, 12000,  4.0, 0.40, fs);
	filters[11] = biquad_gen(FILT_HIGHPASS,    150,  0.0, 1.00, fs);
}
