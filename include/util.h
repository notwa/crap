#include "math.h"

#ifdef __SSE2__
#include <emmintrin.h>
#endif

#define INNER static inline
#define PURE __attribute__((pure))
#define CONST __attribute__((const))

#ifndef FORCE_SINGLE
typedef double v2df __attribute__((vector_size(16), aligned(16)));
#else
typedef float v2df __attribute__((vector_size(8), aligned(8)));
#endif

typedef float v4sf __attribute__((vector_size(16), aligned(16)));
typedef unsigned long ulong; // __attribute((aligned(16)));

#define V(x) (v2df){(x), (x)}

INNER void
disable_denormals();

#define LIMIT(v,l,u) ((v)<(l)?(l):((v)>(u)?(u):(v)))
#define DB2LIN(x) ((x) > -90 ? pow(10, (x) * 0.05) : 0)

/* frequency to rads/sec (angular frequency) */
#define ANGULAR(fc, fs)     (2 * M_PI / (fs) * (fc))
#define ANGULAR_LIM(fc, fs) (2 * M_PI / (fs) * LIMIT((fc), 1, (fs)/2))

typedef struct {
	double a1, a2, b0, b1, b2, x1, x2, y1, y2;
} biquad;

typedef struct {
	double b0, b1, b2, a0, a1, a2;
} biquad_interim;

INNER float
whitenoise();

INNER void
biquad_init(biquad *bq);

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

INNER biquad
biquad_gen(filter_t type, double fc, double gain, double bw, double fs);

/* s-plane to z-plane */
static biquad_interim
design(double cw, double sw,
    double num0, double num1, double num2,
    double den0, double den1, double den2);

INNER double
biquad_run(biquad *bq, double x);

#include "util_def.h"
