#define DELAY
static auto global_delay = 2;

#define OVERSAMPLE 4

// number of taps. coefficients via:
// http://vladgsound.wordpress.com/2013/06/01/iir-based-eq-and-distortions/
#define UP 19
#define DOWN 5

#include "util.hpp"
#include "Param.hpp"
#include "Crap.hpp"
#include "biquad.hpp"

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

struct channel {
	double up[UP], down[DOWN]; // TODO: dumb in fir_up/fir_down struct
	biquad filter;

	inline double
	process(double s)
	{
		s = fir_down(down, biquad_run(&filter, fir_up(up, s)));
		    fir_down(down, biquad_run(&filter, fir_up(up, 0)));
		    fir_down(down, biquad_run(&filter, fir_up(up, 0)));
		    fir_down(down, biquad_run(&filter, fir_up(up, 0)));
		return s;
	}
};

struct Crap_delay_test
:public AdjustAll<Crap> {
	static constexpr ulong id = 0xDEDEDEDE;
	static constexpr char label[] = "crap_delay_test";
	static constexpr char name[] = "crap sample delay test";
	static constexpr char author[] = "Connor Olding";
	static constexpr char copyright[] = "MIT";
	static constexpr ulong parameters = 0;

	channel c_L, c_R;

	inline void
	process(
	    double *in_L, double *in_R,
	    double *out_L, double *out_R,
	    ulong count)
	{
		for (ulong i = 0; i < count; i++) {
			out_L[i] = c_L.process(in_L[i]);
			out_R[i] = c_R.process(in_R[i]);
		}
	}

	inline void
	process(
	    float *in_L, float *in_R,
	    float *out_L, float *out_R,
	    ulong count)
	{
		for (ulong i = 0; i < count; i++) {
			out_L[i] = c_L.process(in_L[i]);
			out_R[i] = c_R.process(in_R[i]);
		}
	}

	static inline void
	construct_params(Param *params)
	{}

	inline void
	resume()
	{}

	inline void
	pause()
	{}

	inline void
	adjust_all(Param *params)
	{
		for (int i = 0; i < UP; i++)
			c_L.up[i] = 0;
		for (int i = 0; i < DOWN; i++)
			c_L.down[i] = 0;
		c_L.filter = biquad_gen(FILT_PEAKING,
		    16630, 10, 1, fs*OVERSAMPLE);
		biquad_init(&c_L.filter);
		c_R = c_L;
	}
};
