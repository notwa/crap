#include <string.h>
#include <math.h>

#include "util.hpp"
#include "Param.hpp"
#include "Crap.hpp"
#include "Buffer2.hpp"
#include "biquad.hpp"

#define DELAY 1
static ulong global_delay = 0;

TEMPLATE
struct Delay {
	static constexpr ulong size = 4096;

	// needs to be twice the size for a memcpy trick later
	T buf[size*2];
	int length, pos;

	inline void
	setup(int newlen)
	{
		pos = 0;
		length = newlen;
		for (int i = 0; i < size*2; i++)
			buf[i] = T(0);
	};

	inline T
	delay(T s)
	{
		pos--;
		if (pos <= 0) {
			memcpy(buf + size, buf, size*sizeof(T));
			pos = size;
		}
		buf[pos] = s;
		return buf[pos + length];
	};

	inline T &
	operator[](int index) {
		return buf[pos + index];
	}

	inline const T &
	operator[](int index) const {
		return buf[pos + index];
	}
};

struct Crap_level
:public AdjustAll<Buffer2<Crap>> {
	static constexpr ulong id = 0xAAAAAAAA;
	static constexpr char label[] = "crap_level";
	static constexpr char name[] = "crap Leveller";
	static constexpr char author[] = "Connor Olding";
	static constexpr char copyright[] = "MIT";

	static constexpr ulong bands = 2;
	static constexpr ulong parameters = 0;

	v2df sc[BLOCK_SIZE]; // sidechain
	biquad filters_L[bands];
	biquad filters_R[bands];
	ulong window_size;
	ulong lookahead;
	double *window;
	double env, attack, release;
	Delay<v2df> delay;
	Delay<double> window_delay;

	Crap_level()
	{
		window = NULL;
	};

	virtual
	~Crap_level()
	{
		if (window)
			free(window);
	}

	inline void
	process2(v2df *buf, ulong rem)
	{
		biquad *f0, *f1;
		f0 = filters_L;
		f1 = filters_R;
		memcpy(sc, buf, rem*sizeof(v2df));
		for (ulong i = 0; i < bands; i++) {
			biquad_run_block_stereo(f0, f1, sc, rem);
			f0++;
			f1++;
		}
		double envs[rem];
		for (ulong i = 0; i < rem; i++)
			envs[i] = fabs(sc[i][0]) + fabs(sc[i][1]);
		for (ulong i = 0; i < rem; i++)
			envs[i] *= 0.5;

		// MaxFIR
		double applied[window_size];
		for (ulong i = 0; i < rem; i++) {
			window_delay.delay(envs[i]);
			for (ulong j = 0; j < window_size; j++)
				applied[j] = window[j]*window_delay[j];
			double max_ = 0;
			for (ulong j = 0; j < window_size; j++)
				max_ = fmax(max_, applied[j]);
			envs[i] = max_;
		}

		// Follower
		for (ulong i = 0; i < rem; i++) {
			env += (envs[i] - env)*(env < envs[i] ? attack : release);
			env = fmax(0.00001, env);
			envs[i] = env;
		}
		double gains[rem];
		for (ulong i = 0; i < rem; i++)
			gains[i] = 0.3991 - 0.01769/(envs[i] + 0.044);
		for (ulong i = 0; i < rem; i++)
			gains[i] = fmax(0, gains[i]);
		for (ulong i = 0; i < rem; i++)
			gains[i] /= envs[i];

		for (ulong i = 0; i < rem; i++) {
			v2df gain = v2df(gains[i]);
			buf[i] = delay.delay(buf[i])*gain;
		}
	}

	inline void
	pause()
	{}

	inline void
	resume()
	{
		for (int i = 0; i < bands; i++) {
			biquad_init(&filters_L[i]);
			biquad_init(&filters_R[i]);
		}
		env = 0;
		delay.setup(lookahead);
		window_delay.setup(window_size);
	}

	static inline void
	construct_params(Param *params)
	{}

	virtual void
	adjust_all(Param *params)
	{
		biquad *f = filters_L;
		f[ 0] = biquad_gen(FILT_PEAKING,  50.0,  -10, 4.00, fs);
		f[ 1] = biquad_gen(FILT_PEAKING,  5000,    5, 4.00, fs);
		for (int i = 0; i < bands; i++)
			filters_R[i] = filters_L[i];

		double window_length = fs*0.010/0.60;
		window_size = round(window_length);
		lookahead = round(window_length*0.6);
		global_delay = lookahead;

		if (window)
			free(window);
		window = (double *) calloc(window_size, sizeof(double));

		for (int i = 0; i < window_size; i++) {
			double x = double(i)/window_size;
			double y = -(x - 0)*(x - 1)*(x + 0.6)/0.288;
			window[i] = y;
		}

		attack  = 1 - exp(-1/(fs*0.002));
		release = 1 - exp(-1/(fs*0.050));

		resume();
	}
};

constexpr char Crap_level::label[];
constexpr char Crap_level::name[];
constexpr char Crap_level::author[];
constexpr char Crap_level::copyright[];
