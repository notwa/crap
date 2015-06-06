#include <string.h>

#define ID (0x0DEFACED+420)
#define LABEL "crap_eq_const_T420"
#define NAME "crap T420 Speaker Compensation"
#define AUTHOR "Connor Olding"
#define COPYRIGHT "MIT"
#define PARAMETERS 0

#define BLOCK_SIZE 256

#include "util.hpp"

#define BANDS 16
typedef struct {
	biquad filters[2][BANDS];
} personal;

template<typename T>
static void
process(personal *data,
    T *in_L, T *in_R,
    T *out_L, T *out_R,
    ulong count)
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
	filters[ 3] = biquad_gen(FILT_PEAKING,    1580,  7.0, 0.25, fs);
	filters[ 4] = biquad_gen(FILT_PEAKING,    2080, -2.5, 0.30, fs);
	filters[ 5] = biquad_gen(FILT_PEAKING,    2270,  6.0, 0.20, fs);
	filters[ 6] = biquad_gen(FILT_PEAKING,    2470, -2.0, 0.18, fs);
	filters[ 7] = biquad_gen(FILT_PEAKING,    3700, -5.0, 0.32, fs);
	filters[ 8] = biquad_gen(FILT_PEAKING,    6200, -3.5, 0.25, fs);
	filters[ 9] = biquad_gen(FILT_PEAKING,    6000, -11., 3.66, fs);
	filters[10] = biquad_gen(FILT_HIGHSHELF, 11500,  4.0, 0.40, fs);
	filters[11] = biquad_gen(FILT_HIGHPASS,    150,  0.0, 1.00, fs);
	filters[12] = biquad_gen(FILT_PEAKING,    1775, -2.0, 0.18, fs);
	filters[13] = biquad_gen(FILT_PEAKING,     490, -1.5, 0.23, fs);
	filters[14] = biquad_gen(FILT_PEAKING,    3100,  5.0, 0.33, fs);
	filters[15] = biquad_gen(FILT_LOWPASS,   14000,  0.0, 0.40, fs);
}
