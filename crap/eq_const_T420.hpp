#include <string.h>

#include "util.hpp"
#include "Param.hpp"
#include "Crap.hpp"
#include "Buffer2.hpp"
#include "biquad.hpp"

struct Crap_eq_const_T420
:public AdjustAll<Buffer2<Crap>> {
	static const ulong id = 0x0DEFACED + 420;
	static const char *label;
	static const char *name;
	static const char *author;
	static const char *copyright;
	static const ulong parameters = 0;

	static const ulong bands = 16;

	biquad filters_L[bands];
	biquad filters_R[bands];

	inline void
	process2(v2df *buf, ulong rem)
	{
		biquad *f0, *f1;
		f0 = filters_L;
		f1 = filters_R;
		for (ulong i = 0; i < bands; i++) {
			biquad_run_block_stereo(f0, f1, buf, rem);
			f0++;
			f1++;
		}
	}

	inline void
	pause()
	{}

	inline void
	resume()
	{
		for (int i = 0; i < bands; i++) {
			biquad_init(&filters_L[i]);
			biquad_init(&filters_R[i]);
		}
	}

	static inline void
	construct_params(Param *params)
	{}

	virtual void
	adjust_all(Param *params)
	{
		biquad *f = filters_L;
		f[ 0] = biquad_gen(FILT_PEAKING,    180.,  11., 1.40, fs);
		f[ 1] = biquad_gen(FILT_PEAKING,    740.,  5.5, 0.70, fs);
		f[ 2] = biquad_gen(FILT_PEAKING,    1220, -12., 0.70, fs);
		f[ 3] = biquad_gen(FILT_PEAKING,    1580,  7.0, 0.25, fs);
		f[ 4] = biquad_gen(FILT_PEAKING,    2080, -2.5, 0.30, fs);
		f[ 5] = biquad_gen(FILT_PEAKING,    2270,  6.0, 0.20, fs);
		f[ 6] = biquad_gen(FILT_PEAKING,    2470, -2.0, 0.18, fs);
		f[ 7] = biquad_gen(FILT_PEAKING,    3700, -5.0, 0.32, fs);
		f[ 8] = biquad_gen(FILT_PEAKING,    6200, -3.5, 0.25, fs);
		f[ 9] = biquad_gen(FILT_PEAKING,    6000, -11., 3.66, fs);
		f[10] = biquad_gen(FILT_HIGHSHELF, 11500,  4.0, 0.40, fs);
		f[11] = biquad_gen(FILT_HIGHPASS,    150,  0.0, 1.00, fs);
		f[12] = biquad_gen(FILT_PEAKING,    1775, -2.0, 0.18, fs);
		f[13] = biquad_gen(FILT_PEAKING,     490, -1.5, 0.23, fs);
		f[14] = biquad_gen(FILT_PEAKING,    3100,  5.0, 0.33, fs);
		f[15] = biquad_gen(FILT_LOWPASS,   14000,  0.0, 0.40, fs);
		for (int i = 0; i < bands; i++)
			filters_R[i] = filters_L[i];
	}
};

const char *Crap_eq_const_T420::label = "crap_eq_const_T420";
const char *Crap_eq_const_T420::name = "crap T420 Speaker Compensation";
const char *Crap_eq_const_T420::author = "Connor Olding";
const char *Crap_eq_const_T420::copyright = "MIT";
