#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#include <Eigen/Core>

struct Aligned {
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

#ifndef M_PI
#define M_PI            3.14159265358979323846
#define M_SQRT2         1.41421356237309504880
#define M_SQRT1_2       0.707106781186547524401
#endif

#ifdef __SSE2__
#include <emmintrin.h>
#endif

#define TEMPLATE template<typename T>
#define INNER static inline
#ifdef _MSC_VER
#define PURE
#define CONST
#define RESTRICT
#else
#define PURE __attribute__((pure))
#define CONST __attribute__((const))
#define RESTRICT __restrict__
#endif

typedef unsigned long ulong; // __attribute((aligned(16)));

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 256
#endif

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
INNER float
whitenoise(unsigned int &mirand)
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
