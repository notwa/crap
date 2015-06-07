#define ID 0xDEDEDEDE
#define LABEL "crap_delay_test"
#define NAME "crap sample delay test"
#define AUTHOR "Connor Olding"
#define COPYRIGHT "MIT"
#define PARAMETERS 0
#define DELAY

#include "util.hpp"
#include "biquad.hpp"

static ulong global_delay = 2;
static double oversample = 4;

// taps. coefficients via:
// http://vladgsound.wordpress.com/2013/06/01/iir-based-eq-and-distortions/
#define UP 19
#define DOWN 5

typedef struct {
	double up[UP], down[DOWN];
	biquad filter;
} channel;

typedef struct {
	channel c[2];
} personal;

INNER double
fir_up(double *x, double s)
{
	x[0] = s;
	s = 0.021484375000000000*x[18]
	  + 0.026041666666666668*x[17]
	  - 0.017578125000000000*x[16]
	  - 0.062499999999999972*x[15] // uh
	  - 0.18457031250000000 *x[14]
	  - 0.18229166666666666 *x[13]
	  + 0.14355468750000000 *x[12]
	  + 0.56250000000000000 *x[11]
	  + 1.0371093750000000  *x[10]
	  + 1.3125000000000000  *x[9]
	  + 1.0371093750000000  *x[8]
	  + 0.56250000000000000 *x[7]
	  + 0.14355468750000000 *x[6]
	  - 0.18229166666666666 *x[5]
	  - 0.18457031250000000 *x[4]
	  - 0.062499999999999993*x[3] // uh
	  - 0.017578125000000000*x[2]
	  + 0.026041666666666668*x[1]
	  + 0.021484375000000000*x[0];
	for (int i = UP - 1; i > 0; i--)
		x[i] = x[i - 1];
	return s;
}

INNER double
fir_down(double *x, double s)
{
	x[0] = s;
	s = 0.077777777777777779*x[4]
	  + 0.35555555555555557 *x[3]
	  + 0.13333333333333333 *x[2]
	  + 0.35555555555555557 *x[1]
	  + 0.077777777777777779*x[0];
	for (int i = DOWN - 1; i > 0; i--)
		x[i] = x[i - 1];
	return s;
}

INNER double
process_one(channel *c, double s)
{
	s =    fir_down(c->down, biquad_run(&c->filter, fir_up(c->up, s)));
	       fir_down(c->down, biquad_run(&c->filter, fir_up(c->up, 0)));
	       fir_down(c->down, biquad_run(&c->filter, fir_up(c->up, 0)));
	       fir_down(c->down, biquad_run(&c->filter, fir_up(c->up, 0)));
	return s;
}

template<typename T>
INNER void
process(personal *data,
    T *in_L, T *in_R,
    T *out_L, T *out_R,
    ulong count)
{
	for (ulong pos = 0; pos < count; pos++) {
		out_L[pos] = process_one(&data->c[0], in_L[pos]);
		out_R[pos] = process_one(&data->c[1], in_R[pos]);
	}
}

INNER void
construct(personal *data)
{}

INNER void
destruct(personal *data)
{}

INNER void
resume(personal *data)
{}

INNER void
pause(personal *data)
{}

INNER void
adjust(personal *data, ulong fs)
{
	for (int k = 0; k < 2; k++) {
		channel *c = &data->c[k];
		for (int i = 0; i < UP; i++)
			c->up[i] = 0;
		for (int i = 0; i < DOWN; i++)
			c->down[i] = 0;
		c->filter = biquad_gen(FILT_PEAKING,
		    16630, 10, 1, fs*oversample);
		biquad_init(&c->filter);
	}
}
