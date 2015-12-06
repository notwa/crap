#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "util.hpp"
#include "Param.hpp"
#include "Crap.hpp"
#include "svf.hpp"
#include "Buffer4.hpp"

struct Crap_eq_const_T420_svf
:public AdjustAll<Buffer4<Crap>> {
	static const ulong id = 0x0DEFACED + 420 + 1337;
	static const char *label;
	static const char *name;
	static const char *author;
	static const char *copyright;
	static const ulong parameters = 0;

	static const ulong bands = 16;

	svf_matrix<v4sf> filters_L[bands];
	svf_matrix<v4sf> filters_R[bands];

	inline void
	process2(v4sf *buf_L, v4sf *buf_R, ulong rem)
	{
		svf_matrix<v4sf> *f0, *f1;
		f0 = filters_L;
		f1 = filters_R;

		for (ulong i = 0; i < bands; i++) {
			svf_run_block_mat(f0, buf_L, rem);
			f0++;
		}
		for (ulong i = 0; i < bands; i++) {
			svf_run_block_mat(f1, buf_R, rem);
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
			filters_L[i].memory = v4sf(0);
			filters_R[i].memory = v4sf(0);
		}
	}

	static inline void
	construct_params(Param *params)
	{}

	inline void
	adjust_all(Param *params)
	{
		svf_matrix<v4sf> *f = filters_L;
		#define gen(a, b, c, d, e) svf_gen_matrix<v4sf>(svf_gen(a, b, c, d, e))
		f[ 0] = gen(FILT_PEAKING,    180.,  11., 1.40, fs);
		f[ 1] = gen(FILT_PEAKING,    740.,  5.5, 0.70, fs);
		f[ 2] = gen(FILT_PEAKING,    1220, -12., 0.70, fs);
		f[ 3] = gen(FILT_PEAKING,    1580,  7.0, 0.25, fs);
		f[ 4] = gen(FILT_PEAKING,    2080, -2.5, 0.30, fs);
		f[ 5] = gen(FILT_PEAKING,    2270,  6.0, 0.20, fs);
		f[ 6] = gen(FILT_PEAKING,    2470, -2.0, 0.18, fs);
		f[ 7] = gen(FILT_PEAKING,    3700, -5.0, 0.32, fs);
		f[ 8] = gen(FILT_PEAKING,    6200, -3.5, 0.25, fs);
		f[ 9] = gen(FILT_PEAKING,    6000, -11., 3.66, fs);
		f[10] = gen(FILT_HIGHSHELF, 11500,  4.0, 0.40, fs);
		f[11] = gen(FILT_HIGHPASS,    150,  0.0, 1.00, fs);
		f[12] = gen(FILT_PEAKING,    1775, -2.0, 0.18, fs);
		f[13] = gen(FILT_PEAKING,     490, -1.5, 0.23, fs);
		f[14] = gen(FILT_PEAKING,    3100,  5.0, 0.33, fs);
		f[15] = gen(FILT_LOWPASS,   14000,  0.0, 0.40, fs);
		#undef gen
		for (int i = 0; i < bands; i++)
			filters_R[i] = filters_L[i];
	}
};

const char *Crap_eq_const_T420_svf::label = "crap_eq_const_T420_svf";
const char *Crap_eq_const_T420_svf::name = "crap T420 Speaker Compensation (SVF)";
const char *Crap_eq_const_T420_svf::author = "Connor Olding";
const char *Crap_eq_const_T420_svf::copyright = "MIT";
