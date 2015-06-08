#include <string.h>

#include "util.hpp"
#include "Param.hpp"
#include "Crap.hpp"

struct Crap_noise
:public AdjustAll<Crap> {
	static constexpr ulong id = 0xEC57A71C;
	static constexpr char label[] = "crap_noise";
	static constexpr char name[] = "crap noise generator";
	static constexpr char author[] = "Connor Olding";
	static constexpr char copyright[] = "MIT";
	static constexpr ulong parameters = 0;

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
