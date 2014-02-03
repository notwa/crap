#include "math.h"

#define LIMIT(v,l,u) ((v)<(l)?(l):((v)>(u)?(u):(v)))
#define DB2LIN(x) ((x) > -90 ? pow(10, (x) * 0.05) : 0)

/* frequency to rads/sec (angular frequency) */
#define ANGULAR(fc, fs)     (2 * M_PI / (fs) * (fc))
#define ANGULAR_LIM(fc, fs) (2 * M_PI / (fs) * LIMIT((fc), 1, (fs)/2))

/* this is pretty gross;
 * it's too easy to define BIQUAD_DOUBLE in one file and not another */
#ifdef BIQUAD_DOUBLE
typedef double bq_t;
#else
typedef float bq_t;
#endif

typedef struct {
	bq_t a1, a2, b0, b1, b2, x1, x2, y1, y2;
} biquad;

typedef struct {
	double b0, b1, b2, a0, a1, a2;
} biquad_interim;

static float
whitenoise();

static void
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

static biquad
biquad_gen(filter_t type, double fc, double gain, double bw, double fs);

/* s-plane to z-plane */
static biquad_interim
design(double cw, double sw,
    double num0, double num1, double num2,
    double den0, double den1, double den2);

static bq_t
biquad_run(biquad *bq, bq_t x);

#include "crap_util_def.h"
