#include <string.h>

#include "util.hpp"
#include "Param.hpp"
#include "Crap.hpp"

struct Crap_noise
:public AdjustAll<Crap> {
	static const ulong id = 0xEC57A71C;
	static const char *label;
	static const char *name;
	static const char *author;
	static const char *copyright;
	static const ulong parameters = 0;

	unsigned int mirand_L, mirand_R;

	inline
	Crap_noise()
	: mirand_L(123), mirand_R(456)
	{}

	inline void
	process(
	    double *in_L, double *in_R,
	    double *out_L, double *out_R,
	    ulong count)
	{
		for (ulong i = 0; i < count; i++)
			out_L[i] = whitenoise(mirand_L);
		for (ulong i = 0; i < count; i++)
			out_R[i] = whitenoise(mirand_R);
	}

	inline void
	process(
	    float *in_L, float *in_R,
	    float *out_L, float *out_R,
	    ulong count)
	{
		for (ulong i = 0; i < count; i++)
			out_L[i] = whitenoise(mirand_L);
		for (ulong i = 0; i < count; i++)
			out_R[i] = whitenoise(mirand_R);
	}

	static inline void
	construct_params(Param *params)
	{}

	inline void
	resume()
	{}

	inline void
	pause()
	{}

	inline void
	adjust_all(Param *params)
	{}
};

const char *Crap_noise::label = "crap_noise";
const char *Crap_noise::name = "crap noise generator";
const char *Crap_noise::author = "Connor Olding";
const char *Crap_noise::copyright = "MIT";
