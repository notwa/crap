#include <string.h>

#define ID 0x0DEFACED
#define LABEL "crap_eq_const"
#define NAME "crap Constant Equalizer"
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
    ulong count)
{
	disable_denormals();

	double buf[2*BLOCK_SIZE];

	biquad *f0, *f1;

	for (ulong pos = 0; pos < count; pos += BLOCK_SIZE) {
		ulong rem = BLOCK_SIZE;
		if (pos + BLOCK_SIZE > count)
			rem = count - pos;

		for (ulong i = 0; i < rem; i++) {
			buf[i*2+0] = in_L[i];
			buf[i*2+1] = in_R[i];
		}

		f0 = data->filters[0];
		f1 = data->filters[1];
		for (ulong i = 0; i < BANDS; i++) {
			biquad_run_block_stereo(f0, f1, buf, rem);
			f0++;
			f1++;
		}

		for (ulong i = 0; i < rem; i++) {
			out_L[i] = buf[i*2+0];
			out_R[i] = buf[i*2+1];
		}

		in_L += BLOCK_SIZE;
		in_R += BLOCK_SIZE;
		out_L += BLOCK_SIZE;
		out_R += BLOCK_SIZE;
	}
}

#include "process.h"

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
	filters[ 0] = biquad_gen(FILT_PEAKING,  62.0,  5.3, 0.55, fs);
	filters[ 1] = biquad_gen(FILT_PEAKING,  72.0, -1.7, 1.92, fs);
	filters[ 2] = biquad_gen(FILT_PEAKING,  2070, -3.1, 0.10, fs);
	filters[ 3] = biquad_gen(FILT_PEAKING,  2324,  1.9, 0.96, fs);
	filters[ 4] = biquad_gen(FILT_PEAKING,  2340, -3.9, 0.12, fs);
	filters[ 5] = biquad_gen(FILT_PEAKING,  3346,  1.4, 0.25, fs);
	filters[ 6] = biquad_gen(FILT_PEAKING,  4800, -3.4, 0.24, fs);
	filters[ 7] = biquad_gen(FILT_PEAKING,  5734,  1.7, 0.13, fs);
	filters[ 8] = biquad_gen(FILT_PEAKING,  6046,  1.0, 0.11, fs);
	filters[ 9] = biquad_gen(FILT_PEAKING,  6300, -6.4, 1.00, fs);
	filters[10] = biquad_gen(FILT_PEAKING,  8390,  1.5, 0.15, fs);
	filters[11] = biquad_gen(FILT_PEAKING, 13333,  3.1, 0.19, fs);
}
