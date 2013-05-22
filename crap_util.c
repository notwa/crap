#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#include "crap_util.h"

/* used to resemble https://github.com/swh/ladspa/blob/master/util/biquad.h */

void
biquad_init(biquad *bq) {
	bq->x1 = 0;
	bq->x2 = 0;
	bq->y1 = 0;
	bq->y2 = 0;
}

biquad_interim
peaking(double cw, double Gf, double g) {
	double GB = sqrt(Gf);
	return (biquad_interim) {
		.b0 = 1 + g * GB,
		.b1 = -2 * cw,
		.b2 = 1 - g * GB,
		.a0 = 1 + g / GB,
		.a1 = -2 * cw,
		.a2 = 1 - g / GB
	};
}

biquad_interim
highpass(double cw, double g) {
	return (biquad_interim) {
		.b0 = 1 + cw*0.5,
		.b1 = -cw - 1,
		.b2 = 1 + cw*0.5,
		.a0 = 1 + g,
		.a1 = -2*cw,
		.a2 = 1 - g
	};
}

/* TODO: rename */
biquad_interim
orfanidi(double w0, double Gf, double g) {
	/* simplified http://ece.rutgers.edu/~orfanidi/intro2sp/mdir/peq.m */
	double Dw, GfS, F, G00, F00, term1, term2, num, den;
	double G1S, G1, G01, G11, F01, F11, W2, DO, coeff, C, D, A, B;
	double v;

	Dw = atan(g)*2;
	GfS = SQR(Gf);

	v = (Gf > 1) ? 1 : -1;

	F   = v*(GfS - Gf);
	G00 = v*(GfS - 1);
	F00 = v*(Gf - 1);

	term1 = SQR(SQR(w0) - SQR(M_PI));
	term2 = F00 * SQR(M_PI) * SQR(Dw) / F;
	num = term1 + term2 * GfS;
	den = term1 + term2;

	G1S = num/den;
	G1 = sqrt(G1S);

	G01 = v*(GfS - G1);
	G11 = v*(GfS - G1S);
	F01 = v*(Gf - G1);
	F11 = fabs(Gf - G1S);

	/* bandwidth compensation goes nuts
	 * skip |= v*(Gf - G1S) < 0; */

	W2 = sqrt(G11 / G00) * SQR(tan(w0/2));
	DO = (1 + sqrt(F00 / F11) * W2) * g;

	coeff = 2 * W2 / F;
	C = F11 * SQR(DO) / F - coeff * (F01 - sqrt(F00 * F11));
	D =                     coeff * (G01 - sqrt(G00 * G11));

	A = sqrt(      C +      D);
	B = sqrt(GfS * C + Gf * D);

	return (biquad_interim) {
		.b0 = -(G1 + W2 + B),
		.b1 = 2*(G1 - W2),
		.b2 = -(G1 - B + W2),
		.a0 = -(1 + W2 + A),
		.a1 = 2*(1 - W2),
		.a2 = -(1 + W2 - A)
	};
}

void
biquad_gen(biquad *bq, int type, double fc, double gain, double bw, double fs) {
	/* TODO: use enum for type instead of just int */
	double w0, cw, sw, Gf, Q, a_peak, a_pass;
	w0 = ANGULAR_LIM(fc, fs);
	cw = cos(w0);
	sw = sin(w0);
	Gf = DB2LIN(gain);
	Q = 1/(2 * sinh(LN_2_2 * bw * w0/sw));

	a_peak = sw * (2 / Q);
	a_pass = sw / (1 * Q);
	/* skip = (fabs(Gf - 1) <= TINY); */
	biquad_interim bqi;
	if (type == 0) bqi = peaking(cw, Gf, a_peak);
	if (type == 1) bqi = orfanidi(w0, Gf, a_peak);
	if (type == 2) bqi = highpass(cw, a_pass);

	double a0r = 1/bqi.a0;
	bq->b0 = a0r * bqi.b0;
	bq->b1 = a0r * bqi.b1;
	bq->b2 = a0r * bqi.b2;
	bq->a1 = -a0r * bqi.a1;
	bq->a2 = -a0r * bqi.a2;
}

bq_t
biquad_run(biquad *bq, bq_t x) {
	bq_t y;

	y = bq->b0 * x + bq->b1 * bq->x1 + bq->b2 * bq->x2
	               + bq->a1 * bq->y1 + bq->a2 * bq->y2;
	if (IS_DENORMAL(y)) y = 0;
	bq->x2 = bq->x1;
	bq->x1 = x;
	bq->y2 = bq->y1;
	bq->y1 = y;

	return y;
}

