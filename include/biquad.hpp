/* used to resemble https://github.com/swh/ladspa/blob/master/util/biquad.h */

typedef struct {
	double a1, a2, b0, b1, b2, x1, x2, y1, y2;
} biquad;

typedef struct {
	double b0, b1, b2, a0, a1, a2;
} biquad_interim;

INNER void
biquad_init(biquad *bq)
{
	bq->x1 = bq->x2 = bq->y1 = bq->y2 = 0;
}

static biquad_interim
design(double cw, double sw,
    double num0, double num1, double num2,
    double den0, double den1, double den2)
{
	const biquad_interim ret = {
		num0*   (1 + cw) + num1*sw + num2*  (1 - cw),
		num0*-2*(1 + cw)           + num2*2*(1 - cw),
		num0*   (1 + cw) - num1*sw + num2*  (1 - cw),
		den0*   (1 + cw) + den1*sw + den2*  (1 - cw),
		den0*-2*(1 + cw)           + den2*2*(1 - cw),
		den0*   (1 + cw) - den1*sw + den2*  (1 - cw),
	};
	return ret;
}

// TODO: rename to biquad_gen_raw, fix up parameters like you did with svf
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
	// TODO: error or something
	default:              d(1,    1, 1,   1,      1,   1); break;
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

TEMPLATE INNER void
biquad_run_block_stereo(biquad *bq_L, biquad *bq_R,
    T *buf, ulong count)
{
	T b0, b1, b2, a1, a2, x1, x2, y1, y2;

	b0 = T(bq_L->b0);
	b1 = T(bq_L->b1);
	b2 = T(bq_L->b2);
	a1 = T(bq_L->a1);
	a2 = T(bq_L->a2);

	x1 = T(bq_L->x1, bq_R->x1);
	x2 = T(bq_L->x2, bq_R->x2);
	y1 = T(bq_L->y1, bq_R->y1);
	y2 = T(bq_L->y2, bq_R->y2);

	for (ulong i = 0; i < count; i++) {
		T x = buf[i];
		T y = b0*x + b1*x1 + b2*x2 + a1*y1 + a2*y2;
		x2 = x1;
		y2 = y1;
		x1 = x;
		y1 = y;
		buf[i] = y;
	}

	bq_L->x1 = x1[0];
	bq_R->x1 = x1[1];
	bq_L->x2 = x2[0];
	bq_R->x2 = x2[1];
	bq_L->y1 = y1[0];
	bq_R->y1 = y1[1];
	bq_L->y2 = y2[0];
	bq_R->y2 = y2[1];
}
