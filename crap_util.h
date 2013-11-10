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

static void
biquad_init(biquad *bq);

/* types: TODO: enum
 0: peaking
 1: lowshelf
 2: highshelf
 3: lowpass
 4: highpass
 5: allpass
 6: bandpass 1
 7: bandpass 2
 8: notch
 9: gain
*/
static biquad
biquad_gen(int type, double fc, double gain, double bw, double fs);

/* s-plane to z-plane */
static biquad_interim
design(double cw, double sw,
    double num0, double num1, double num2,
    double den0, double den1, double den2);

static bq_t
biquad_run(biquad *bq, bq_t x);

#include "crap_util_def.h"
