#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

#ifndef M_LN2
#define M_LN2 0.69314718055994530942
#endif

/* ln(2)/2 */
#define LN_2_2 0.3465735902799726547086

#define SQR(x) ((x)*(x))
#define DB2LIN(x) ((x) > -90 ? pow(10, (x) * 0.05) : 0)

/* branchless, supposedly kills denormals when l=1 u=1 */
#define CLIP(v,l,u) (fabs(v-l)+(l+u)-fabs(v-u))*0.5;
/* branches, but smaller and generic with no side effects */
#define LIMIT(v,l,u) ((v)<(l)?(l):((v)>(u)?(u):(v)))

/* frequency to rads/sec (angular frequency) */
#define ANGULAR(fc, fs)     (2 * M_PI / (fs) * (fc))
#define ANGULAR_LIM(fc, fs) (2 * M_PI / (fs) * LIMIT((fc), 1, (fs)/2))

/* http://musicdsp.org/showone.php?id=51 */
/* http://musicdsp.org/files/denormal.pdf */
#ifdef BIQUAD_DOUBLE
typedef double bq_t;
#define IS_DENORMAL(f) (((*(uint64_t *)&f)&0x7FF0000000000000)==0)
#else
typedef float bq_t;
#define IS_DENORMAL(f) (((*(uint32_t *)&f)&0x7F800000)==0)
#endif

typedef struct {
	bq_t a1, a2, b0, b1, b2, x1, x2, y1, y2;
} biquad;

typedef struct {
	double b0, b1, b2, a0, a1, a2;
} biquad_interim;

void
biquad_init(biquad *bq);

biquad_interim
peaking(double cw, double Gf, double g);

biquad_interim
highpass(double cw, double g);

biquad_interim
orfanidi(double w0, double Gf, double g);

void
biquad_gen(biquad *bq, int type, double fc, double gain, double bw, double fs);

bq_t
biquad_run(biquad *bq, bq_t x);

