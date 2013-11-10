#include <stdlib.h>
#include <math.h>
#include <stdint.h>

/* used to resemble https://github.com/swh/ladspa/blob/master/util/biquad.h */

static void
biquad_init(biquad *bq) {
	bq->x1 = bq->x2 = bq->y1 = bq->y2 = 0;
}

static biquad_interim
design(double cw, double sw,
    double num0, double num1, double num2,
    double den0, double den1, double den2) {
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
biquad_gen(int type, double fc, double gain, double bw, double fs) {
	double w0, cw, sw, A, As, Q;
	w0 = ANGULAR_LIM(fc, fs);
	cw = cos(w0);
	sw = sin(w0);
	A = DB2LIN(gain/2);
	As = sqrt(A);
	Q = 1/(4*sinh(M_LN2/2*bw*w0/sw));
	//Q = M_SQRT1_2*(1 - SQR(w0/M_PI))/bw;
	/* skip = (fabs(A - 1) <= TINY); */

	/* TODO: use enum for type instead of just int */
	biquad_interim bqi;
	if (type == 0) bqi = design(cw,sw, 1,  A/Q, 1,   1,  1/A/Q,   1);
	if (type == 1) bqi = design(cw,sw, 1, As/Q, A,   1, 1/As/Q, 1/A);
	if (type == 2) bqi = design(cw,sw, A, As/Q, 1, 1/A, 1/As/Q,   1);
	if (type == 3) bqi = design(cw,sw, 0,    0, 1,   1,    1/Q,   1);
	if (type == 4) bqi = design(cw,sw, 1,    0, 0,   1,    1/Q,   1);
	if (type == 5) bqi = design(cw,sw, 1, -1/Q, 1,   1,    1/Q,   1);
	if (type == 6) bqi = design(cw,sw, 0,    1, 0,   1,    1/Q,   1);
	if (type == 7) bqi = design(cw,sw, 0,  1/Q, 0,   1,    1/Q,   1);
	if (type == 8) bqi = design(cw,sw, 1,    0, 1,   1,    1/Q,   1);
	if (type == 9) bqi = design(cw,sw, A,    A, A, 1/A,    1/A, 1/A);

	double a0r = 1/bqi.a0;

	return (biquad) {
		.b0 = a0r * bqi.b0,
		.b1 = a0r * bqi.b1,
		.b2 = a0r * bqi.b2,
		.a1 = -a0r * bqi.a1,
		.a2 = -a0r * bqi.a2
	};
}

static bq_t
biquad_run(biquad *bq, bq_t x) {
	bq_t y;

	y = bq->b0 * x + bq->b1 * bq->x1 + bq->b2 * bq->x2
	               + bq->a1 * bq->y1 + bq->a2 * bq->y2;
	bq->x2 = bq->x1;
	bq->x1 = x;
	bq->y2 = bq->y1;
	bq->y1 = y;

	return y;
}
