#include <string.h>
#include <stdio.h>

#define ID (0x0DEFACED+420+1337)
#define LABEL "crap_eq_const_T420_svf"
#define NAME "crap T420 Speaker Compensation (SVF)"
#define AUTHOR "Connor Olding"
#define COPYRIGHT "MIT"
#define PARAMETERS 0

#define BLOCK_SIZE 256

#include "util.h"

#define BANDS 16
typedef struct {
	svf filters[2][BANDS];
} personal;

static void
process_double(personal *data,
    double *in_L, double *in_R,
    double *out_L, double *out_R,
    unsigned long count)
#include "process_svfs.h"

static void
process(personal *data,
    float *in_L, float *in_R,
    float *out_L, float *out_R,
    ulong count)
#include "process_svfs.h"

INNER void
construct(personal *data)
{}

INNER void
destruct(personal *data)
{}

INNER void
resume(personal *data)
{
	svf *filters = data->filters[0];
	for (int i = 0; i < BANDS; i++) {
		filters[i].memory[0] = 0;
		filters[i].memory[1] = 0;
	}
	memcpy(data->filters[1], filters, BANDS*sizeof(svf));
}

INNER void
pause(personal *data)
{}

INNER void
adjust(personal *data, ulong fs)
{
	svf *filters = data->filters[0];
	filters[ 0] = svf_gen(FILT_PEAKING,    180.,  11., 1.40, fs);
	filters[ 1] = svf_gen(FILT_PEAKING,    740.,  5.5, 0.70, fs);
	filters[ 2] = svf_gen(FILT_PEAKING,    1220, -12., 0.70, fs);
	filters[ 3] = svf_gen(FILT_PEAKING,    1580,  7.0, 0.25, fs);
	filters[ 4] = svf_gen(FILT_PEAKING,    2080, -2.5, 0.30, fs);
	filters[ 5] = svf_gen(FILT_PEAKING,    2270,  6.0, 0.20, fs);
	filters[ 6] = svf_gen(FILT_PEAKING,    2470, -2.0, 0.18, fs);
	filters[ 7] = svf_gen(FILT_PEAKING,    3700, -5.0, 0.32, fs);
	filters[ 8] = svf_gen(FILT_PEAKING,    6200, -3.5, 0.25, fs);
	filters[ 9] = svf_gen(FILT_PEAKING,    6000, -11., 3.66, fs);
	filters[10] = svf_gen(FILT_HIGHSHELF, 11500,  4.0, 0.40, fs);
	filters[11] = svf_gen(FILT_HIGHPASS,    150,  0.0, 1.00, fs);
	filters[12] = svf_gen(FILT_PEAKING,    1775, -2.0, 0.18, fs);
	filters[13] = svf_gen(FILT_PEAKING,     490, -1.5, 0.23, fs);
	filters[14] = svf_gen(FILT_PEAKING,    3100,  5.0, 0.33, fs);
	filters[15] = svf_gen(FILT_LOWPASS,   14000,  0.0, 0.40, fs);
	/* debugging
	svf *f = &filters[10];
	printf("A = [%.8f %.8f]\n", f->A0[0], f->A0[1]);
	printf("    [%.8f %.8f]\n", f->A1[0], f->A1[1]);
	printf("B = {%.8f, %.8f}\n", f->B[0], f->B[1]);
	printf("C = {%.8f, %.8f, %9.7f}\n", f->C[0], f->C[1], f->C[2]);
	*/
}
