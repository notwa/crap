#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#ifdef __SSE2__
#include <emmintrin.h>
#endif

#define TEMPLATE template<typename T>
#define INNER static inline
#define PURE __attribute__((pure))
#define CONST __attribute__((const))
#define RESTRICT __restrict__

typedef unsigned long ulong; // __attribute((aligned(16)));

#include "Dumber.hpp"
#include "vectors.hpp"

#ifdef FORCE_SINGLE
#define v2df v2sf
#endif

INNER void
disable_denormals()
{
	#if __SSE2__
	_mm_setcsr(_mm_getcsr() | 0x8040);
	#endif
}

#define LIMIT(v,l,u) ((v)<(l)?(l):((v)>(u)?(u):(v)))
#define DB2LIN(x) ((x) > -90 ? pow(10, (x) * 0.05) : 0)

/* frequency to rads/sec (angular frequency) */
#define ANGULAR(fc, fs)     (2 * M_PI / (fs) * (fc))
#define ANGULAR_LIM(fc, fs) (2 * M_PI / (fs) * LIMIT((fc), 1, (fs)/2))

/* via http://www.rgba.org/articles/sfrand/sfrand.htm */
static unsigned int mirand = 1;

INNER float
whitenoise()
{
	union either {
		float f;
		unsigned int i;
	} white;
	mirand *= 16807;
	white.i = (mirand & 0x007FFFFF) | 0x40000000;
	return white.f - 3;
}

typedef enum {
	FILT_PEAKING,
	FILT_LOWSHELF,
	FILT_HIGHSHELF,
	FILT_LOWPASS,
	FILT_HIGHPASS,
	FILT_ALLPASS,
	FILT_BANDPASS,
	FILT_BANDPASS_2,
	FILT_NOTCH,
	FILT_GAIN
} filter_t;
