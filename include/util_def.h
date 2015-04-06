#include <stdlib.h>
#include <math.h>
#include <stdint.h>

INNER void
disable_denormals()
{
	#if __SSE2__
        _mm_setcsr(_mm_getcsr() | 0x8040);
	#endif
}

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

/* used to resemble https://github.com/swh/ladspa/blob/master/util/biquad.h */

INNER void
biquad_init(biquad *bq)
{
	bq->x1 = bq->x2 = bq->y1 = bq->y2 = 0;
}

INNER biquad_interim
design(double cw, double sw,
    double num0, double num1, double num2,
    double den0, double den1, double den2)
{
	return (biquad_interim) {
		.b0 = num0*   (1 + cw) + num1*sw + num2*  (1 - cw),
		.b1 = num0*-2*(1 + cw)           + num2*2*(1 - cw),
		.b2 = num0*   (1 + cw) - num1*sw + num2*  (1 - cw),
		.a0 = den0*   (1 + cw) + den1*sw + den2*  (1 - cw),
		.a1 = den0*-2*(1 + cw)           + den2*2*(1 - cw),
		.a2 = den0*   (1 + cw) - den1*sw + den2*  (1 - cw),
	};
}

static biquad
biquad_gen(filter_t type, double fc, double gain, double bw, double fs)
{
	double w0, cw, sw, A, As, Q;
	w0 = ANGULAR_LIM(fc, fs);
	cw = cos(w0);
	sw = sin(w0);
	A = DB2LIN(gain/2);
	As = sqrt(A);
	Q = M_SQRT1_2*(1 - (w0/M_PI)*(w0/M_PI))/bw;
	/* skip = (fabs(A - 1) <= TINY); */

	biquad_interim bqi;

	#define d(n0,n1,n2,d0,d1,d2) bqi = design(cw,sw,n0,n1,n2,d0,d1,d2)
	switch (type) {
	case FILT_PEAKING:    d(1,  A/Q, 1,   1,  1/A/Q,   1); break;
	case FILT_LOWSHELF:   d(1, As/Q, A,   1, 1/As/Q, 1/A); break;
	case FILT_HIGHSHELF:  d(A, As/Q, 1, 1/A, 1/As/Q,   1); break;
	case FILT_LOWPASS:    d(0,    0, 1,   1,    1/Q,   1); break;
	case FILT_HIGHPASS:   d(1,    0, 0,   1,    1/Q,   1); break;
	case FILT_ALLPASS:    d(1, -1/Q, 1,   1,    1/Q,   1); break;
	case FILT_BANDPASS:   d(0,    1, 0,   1,    1/Q,   1); break;
	case FILT_BANDPASS_2: d(0,  1/Q, 0,   1,    1/Q,   1); break;
	case FILT_NOTCH:      d(1,    0, 1,   1,    1/Q,   1); break;
	case FILT_GAIN:       d(A,    A, A, 1/A,    1/A, 1/A); break;
	}
	#undef d

	double a0r = 1/bqi.a0;

	biquad out;
	out.b0 =  a0r*bqi.b0;
	out.b1 =  a0r*bqi.b1;
	out.b2 =  a0r*bqi.b2;
	out.a1 = -a0r*bqi.a1;
	out.a2 = -a0r*bqi.a2;
	return out;
}

INNER double
biquad_run(biquad *bq, double x)
{
	double y;

	y = bq->b0*x + bq->b1*bq->x1 + bq->b2*bq->x2
	             + bq->a1*bq->y1 + bq->a2*bq->y2;
	bq->x2 = bq->x1;
	bq->x1 = x;
	bq->y2 = bq->y1;
	bq->y1 = y;

	return y;
}

INNER void
biquad_run_block_stereo(biquad *bq_L, biquad *bq_R,
    double *buf, ulong count)
#ifdef __SSE2__
{
	__m128d b0, b1, b2, a1, a2, x1, x2, y1, y2;

	b0 = _mm_set1_pd(bq_L->b0);
	b1 = _mm_set1_pd(bq_L->b1);
	b2 = _mm_set1_pd(bq_L->b2);
	a1 = _mm_set1_pd(bq_L->a1);
	a2 = _mm_set1_pd(bq_L->a2);

	x1 = _mm_setr_pd(bq_L->x1, bq_R->x1);
	x2 = _mm_setr_pd(bq_L->x2, bq_R->x2);
	y1 = _mm_setr_pd(bq_L->y1, bq_R->y1);
	y2 = _mm_setr_pd(bq_L->y2, bq_R->y2);

	for (int i = 0; i < 2*count; i += 2) {
		__m128d x = _mm_load_pd(buf + i);
		__m128d y = b0*x + b1*x1 + b2*x2 + a1*y1 + a2*y2;
		x2 = x1;
		y2 = y1;
		x1 = x;
		y1 = y;
		_mm_store_pd(buf + i, y);
	}

	double temp[8];
	_mm_store_pd(temp+0, x1);
	_mm_store_pd(temp+2, x2);
	_mm_store_pd(temp+4, y1);
	_mm_store_pd(temp+6, y2);
	bq_L->x1 = temp[0];
	bq_R->x1 = temp[1];
	bq_L->x2 = temp[2];
	bq_R->x2 = temp[3];
	bq_L->y1 = temp[4];
	bq_R->y1 = temp[5];
	bq_L->y2 = temp[6];
	bq_R->y2 = temp[7];
}
#else
{
	for (ulong i = 0; i < 2*count; i += 2) {
		buf[i+0] = biquad_run(bq_L, buf[i+0]);
		buf[i+1] = biquad_run(bq_R, buf[i+1]);
	}
}
#endif
