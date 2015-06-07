#define ID (0x0DEFACED+420+1337)
#define LABEL "crap_eq_const_T420_svf"
#define NAME "crap T420 Speaker Compensation (SVF)"
#define AUTHOR "Connor Olding"
#define COPYRIGHT "MIT"
#define PARAMETERS 0

#define BLOCK_SIZE 256

#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "util.hpp"
#include "svf.hpp"

#define BANDS 16
typedef struct {
	svf_matrix<v4sf> filters[2][BANDS];
} personal;

TEMPLATE static void
process(personal *data,
    T *in_L, T *in_R,
    T *out_L, T *out_R,
    unsigned long count)
{
	disable_denormals();

	assert(count % 2 == 0);

	v4sf buf_L[BLOCK_SIZE/2];
	v4sf buf_R[BLOCK_SIZE/2];

	svf_matrix<v4sf> *f0, *f1;

	for (ulong pos = 0; pos < count; pos += BLOCK_SIZE) {
		ulong rem = BLOCK_SIZE;
		if (pos + BLOCK_SIZE > count)
			rem = count - pos;

		for (ulong i = 0, j = 0; i < rem; i += 2, j++) {
			buf_L[j][0] = in_L[i+0];
			buf_L[j][1] = in_L[i+1];
			buf_R[j][0] = in_R[i+0];
			buf_R[j][1] = in_R[i+1];
		}

		f0 = data->filters[0];
		for (ulong i = 0; i < BANDS; i++) {
			svf_run_block_mat(f0, buf_L, rem);
			f0++;
		}
		f1 = data->filters[1];
		for (ulong i = 0; i < BANDS; i++) {
			svf_run_block_mat(f1, buf_R, rem);
			f1++;
		}

		for (ulong i = 0, j = 0; i < rem; i += 2, j++) {
			out_L[i+0] = buf_L[j][0];
			out_L[i+1] = buf_L[j][1];
			out_R[i+0] = buf_R[j][0];
			out_R[i+1] = buf_R[j][1];
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
	svf_matrix<v4sf> *filters = data->filters[0];
	for (int i = 0; i < BANDS; i++)
		filters[i].memory = v4sf(0);
	memcpy(data->filters[1], filters, BANDS*sizeof(svf_matrix<v4sf>));
}

INNER void
pause(personal *data)
{}

INNER void
adjust(personal *data, ulong fs)
{
	svf_matrix<v4sf> *filters = data->filters[0];
	filters[ 0] = svf_gen_matrix<v4sf>(svf_gen(FILT_PEAKING,    180.,  11., 1.40, fs));
	filters[ 1] = svf_gen_matrix<v4sf>(svf_gen(FILT_PEAKING,    740.,  5.5, 0.70, fs));
	filters[ 2] = svf_gen_matrix<v4sf>(svf_gen(FILT_PEAKING,    1220, -12., 0.70, fs));
	filters[ 3] = svf_gen_matrix<v4sf>(svf_gen(FILT_PEAKING,    1580,  7.0, 0.25, fs));
	filters[ 4] = svf_gen_matrix<v4sf>(svf_gen(FILT_PEAKING,    2080, -2.5, 0.30, fs));
	filters[ 5] = svf_gen_matrix<v4sf>(svf_gen(FILT_PEAKING,    2270,  6.0, 0.20, fs));
	filters[ 6] = svf_gen_matrix<v4sf>(svf_gen(FILT_PEAKING,    2470, -2.0, 0.18, fs));
	filters[ 7] = svf_gen_matrix<v4sf>(svf_gen(FILT_PEAKING,    3700, -5.0, 0.32, fs));
	filters[ 8] = svf_gen_matrix<v4sf>(svf_gen(FILT_PEAKING,    6200, -3.5, 0.25, fs));
	filters[ 9] = svf_gen_matrix<v4sf>(svf_gen(FILT_PEAKING,    6000, -11., 3.66, fs));
	filters[10] = svf_gen_matrix<v4sf>(svf_gen(FILT_HIGHSHELF, 11500,  4.0, 0.40, fs));
	filters[11] = svf_gen_matrix<v4sf>(svf_gen(FILT_HIGHPASS,    150,  0.0, 1.00, fs));
	filters[12] = svf_gen_matrix<v4sf>(svf_gen(FILT_PEAKING,    1775, -2.0, 0.18, fs));
	filters[13] = svf_gen_matrix<v4sf>(svf_gen(FILT_PEAKING,     490, -1.5, 0.23, fs));
	filters[14] = svf_gen_matrix<v4sf>(svf_gen(FILT_PEAKING,    3100,  5.0, 0.33, fs));
	filters[15] = svf_gen_matrix<v4sf>(svf_gen(FILT_LOWPASS,   14000,  0.0, 0.40, fs));
}