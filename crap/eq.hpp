#define ID 0x000CAFED
#define LABEL "crap_eq"
#define NAME "crap Parametric Equalizer"
#define AUTHOR "Connor Olding"
#define COPYRIGHT "MIT"
#define BANDS 4
#define PARAMETERS (BANDS*3)

#define BLOCK_SIZE 256

#include <stdio.h>
#include <string.h>

#include "util.hpp"
#include "param.hpp"
#include "Crap.hpp"
#include "biquad.hpp"

template<class Mixin>
struct Buffer2 : public virtual Mixin {
	virtual inline void
	process2(v2df *buf, ulong rem) = 0;

	TEMPLATE void
	_process(T *in_L, T *in_R, T *out_L, T *out_R, ulong count)
	{
		disable_denormals();

		v2df buf[BLOCK_SIZE];

		for (ulong pos = 0; pos < count; pos += BLOCK_SIZE) {
			ulong rem = BLOCK_SIZE;
			if (pos + BLOCK_SIZE > count)
				rem = count - pos;

			for (ulong i = 0; i < rem; i++) {
				buf[i][0] = in_L[i];
				buf[i][1] = in_R[i];
			}

			process2(buf, rem);

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

	void
	process(
	    double *in_L, double *in_R,
	    double *out_L, double *out_R,
	    ulong count)
	{
		_process(in_L, in_R, out_L, out_R, count);
	}

	void
	process(
	    float *in_L, float *in_R,
	    float *out_L, float *out_R,
	    ulong count)
	{
		_process(in_L, in_R, out_L, out_R, count);
	}
};

struct Crap_eq
:public Unconstructive<
	 AdjustAll<
	  Buffer2<Crap>
	 >
	> {
	biquad filters_L[BANDS];
	biquad filters_R[BANDS];

	inline void
	process2(v2df *buf, ulong rem)
	{
		biquad *f0, *f1;
		f0 = filters_L;
		f1 = filters_R;
		for (ulong i = 0; i < BANDS; i++) {
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
		for (int i = 0; i < BANDS; i++) {
			biquad_init(&filters_L[i]);
			biquad_init(&filters_R[i]);
		}
	}

	static inline void
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

	inline void
	adjust_all(param *params)
	{
		for (int i = 0; i < BANDS; i++) {
			filters_L[i] = biquad_gen(FILT_PEAKING,
			    params[0].value, params[1].value, params[2].value, (double) fs);
			params += 3;
			filters_R[i] = filters_L[i];
		}
	}
};
