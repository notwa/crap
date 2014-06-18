/* halfband polyphase IIR filter
   coefficients designed with halfband program:
     https://gist.github.com/3be345efb6c97d757398#file-halfband-c
   parameters: 16 coefficients, 0.1 transition band
   stopband: ~-150dB
   overall delay: ~8 samples
*/

#define copy(dst, src) memcpy(dst, src, sizeof(double)*8)

// all should be initialized to 0
typedef struct {
	double ao[8], bo[8];
	double at[8], bt[8];
	double x1, x2, x3;
	int i;
} halfband_t;

static void
halfband_a(double a[8], double ao[8], double x0, double x2)
{
	a[0] = x2    + (x0   - ao[0])*0.006185967461045014;
	a[1] = ao[0] + (a[0] - ao[1])*0.054230780876613788;
	a[2] = ao[1] + (a[1] - ao[2])*0.143280861566087270;
	a[3] = ao[2] + (a[2] - ao[3])*0.262004358403954640;
	a[4] = ao[3] + (a[3] - ao[4])*0.398796973552973666;
	a[5] = ao[4] + (a[4] - ao[5])*0.545323651071132232;
	a[6] = ao[5] + (a[5] - ao[6])*0.698736833646440347;
	a[7] = ao[6] + (a[6] - ao[7])*0.862917812650502936;
}

static void
halfband_b(double b[8], double bo[8], double x1, double x3)
{
	b[0] = x3    + (x1   - bo[0])*0.024499027624721819;
	b[1] = bo[0] + (b[0] - bo[1])*0.094283481125726432;
	b[2] = bo[1] + (b[1] - bo[2])*0.199699579426327684;
	b[3] = bo[2] + (b[2] - bo[3])*0.328772348316831664;
	b[4] = bo[3] + (b[3] - bo[4])*0.471167216679969414;
	b[5] = bo[4] + (b[4] - bo[5])*0.621096845120503893;
	b[6] = bo[5] + (b[5] - bo[6])*0.778944517099529166;
	b[7] = bo[6] + (b[6] - bo[7])*0.952428157718303137;
}

static double
halfband(halfband_t *h, double x0)
{
	double a[8], b[8];
	halfband_a(a, h->ao,    x0, h->x2);
	halfband_b(b, h->bo, h->x1, h->x3);
	copy(h->ao, h->at);
	copy(h->bo, h->bt);
	copy(h->at, a);
	copy(h->bt, b);
	h->x3 = h->x2;
	h->x2 = h->x1;
	h->x1 = x0;
	return (a[7] + b[7])*0.5;
}

static double
decimate(halfband_t *h, double x0)
{
	double c[8];
	if ((h->i = !h->i)) {
		halfband_b(c, h->bo, x0, h->x2);
		copy(h->bo, c);
		h->x2 = h->x1;
		h->x1 = x0;
		return 0;
	} else {
		halfband_a(c, h->ao, x0, h->x2);
		copy(h->ao, c);
		h->x2 = h->x1;
		h->x1 = x0;
		return (c[7] + h->bo[7])*0.5;
	}
}

// note: do not zero-stuff! send the input each time.
static double
interpolate(halfband_t *h, double x0)
{
	double c[8];
	if ((h->i = !h->i)) {
		halfband_a(c, h->ao, x0, h->x1);
		copy(h->ao, c);
		return c[7];
	} else {
		halfband_b(c, h->bo, x0, h->x1);
		copy(h->bo, c);
		h->x1 = x0;
		return c[7];
	}
}

#undef copy
