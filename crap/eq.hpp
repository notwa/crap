#include <stdio.h>
#include <string.h>

#include "util.hpp"
#include "Param.hpp"
#include "Crap.hpp"
#include "Buffer2.hpp"
#include "biquad.hpp"

struct Crap_eq
:public AdjustAll<Buffer2<Crap>> {
	static const ulong bands = 4;

	static const ulong id = 0x000CAFED;
	static const char *label;
	static const char *name;
	static const char *author;
	static const char *copyright;
	static const ulong parameters = bands*3;

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
	{
		for (int i = 0; i < bands; i++) {
			sprintf(params[0].name, "Band %i Frequency", i + 1);
			params[0].min = 20;
			params[0].max = 20000;
			params[0].scale = SCALE_HZ;
			params[0].def = DEFAULT_440;
			params[0].reset();

			sprintf(params[1].name, "Band %i Gain", i + 1);
			params[1].min = -18;
			params[1].max = 18;
			params[1].scale = SCALE_DB;
			params[1].def = DEFAULT_0;
			params[1].reset();

			sprintf(params[2].name, "Band %i Bandwidth", i + 1);
			params[2].min = 0.0625; // 2^-4 could probably be 2^-3
			params[2].max = 8;      // 2^3  could probably be 2^2
			params[2].scale = SCALE_FLOAT;
			params[2].def = DEFAULT_1;
			params[2].reset();

			params += 3;
		}
	}

	inline void
	adjust_all(Param *params)
	{
		for (int i = 0; i < bands; i++) {
			filters_L[i] = biquad_gen(FILT_PEAKING,
			    params[0].value, params[1].value, params[2].value, (double) fs);
			params += 3;
			filters_R[i] = filters_L[i];
		}
	}
};

const char *Crap_eq::label = "crap_eq";
const char *Crap_eq::name = "crap Parametric Equalizer";
const char *Crap_eq::author = "Connor Olding";
const char *Crap_eq::copyright = "MIT";
