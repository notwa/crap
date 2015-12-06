#include <string.h>

#include "util.hpp"
#include "Param.hpp"
#include "Crap.hpp"
#include "Buffer2.hpp"
#include "biquad.hpp"

struct Crap_eq_const
:public AdjustAll<Buffer2<Crap>> {
	static const ulong id = 0x0DEFACED;
	static const char *label;
	static const char *name;
	static const char *author;
	static const char *copyright;
	static const ulong parameters = 0;

	static const ulong bands = 12;

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
		f[ 0] = biquad_gen(FILT_PEAKING,  62.0,  5.3, 0.55, fs);
		f[ 1] = biquad_gen(FILT_PEAKING,  72.0, -1.7, 1.92, fs);
		f[ 2] = biquad_gen(FILT_PEAKING,  2070, -3.1, 0.10, fs);
		f[ 3] = biquad_gen(FILT_PEAKING,  2324,  1.9, 0.96, fs);
		f[ 4] = biquad_gen(FILT_PEAKING,  2340, -3.9, 0.12, fs);
		f[ 5] = biquad_gen(FILT_PEAKING,  3346,  1.4, 0.25, fs);
		f[ 6] = biquad_gen(FILT_PEAKING,  4800, -3.4, 0.24, fs);
		f[ 7] = biquad_gen(FILT_PEAKING,  5734,  1.7, 0.13, fs);
		f[ 8] = biquad_gen(FILT_PEAKING,  6046,  1.0, 0.11, fs);
		f[ 9] = biquad_gen(FILT_PEAKING,  6300, -6.4, 1.00, fs);
		f[10] = biquad_gen(FILT_PEAKING,  8390,  1.5, 0.15, fs);
		f[11] = biquad_gen(FILT_PEAKING, 13333,  3.1, 0.19, fs);
		for (int i = 0; i < bands; i++)
			filters_R[i] = filters_L[i];
	}
};

const char *Crap_eq_const::label = "crap_eq_const";
const char *Crap_eq_const::name = "crap Constant Equalizer";
const char *Crap_eq_const::author = "Connor Olding";
const char *Crap_eq_const::copyright = "MIT";
