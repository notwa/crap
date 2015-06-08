#define OVERSAMPLING 2
#define BLOCK_SIZE 256
#define FULL_SIZE (BLOCK_SIZE*OVERSAMPLING)

#include <string.h>
#include <stdio.h>

#include "util.hpp"
#include "Param.hpp"
#include "Crap.hpp"
#include "os2piir.hpp"
#include "Buffer2OS2.hpp"

typedef struct {
	double desired, actual, speed;
	int log; // use multiplication instead of addition for speed

	inline double
	smooth()
	{
		if (actual < desired) {
			if (log) actual *= speed;
			else actual += speed;
			if (actual > desired) actual = desired;
		} else if (actual > desired) {
			if (log) actual /= speed;
			else actual -= speed;
			if (actual < desired) actual = desired;
		}
		return actual;
	}
} smoothval;


namespace Tube {
	TEMPLATE INNER CONST T
	distort(T x)
	{
		return (T(27.)*x + T(9.)) / (T(9.)*x*x + T(6.)*x + T(19.)) - T(9./19.);
	}

	TEMPLATE INNER CONST T
	process(T x, T drive, T wet)
	{
		return (distort<T>(x*drive)/drive*T(0.79) - x)*wet + x;
	}
}

struct Crap_tube
:public Buffer2OS2<Crap> {
	static constexpr ulong id = 0x50F7BA11;
	static constexpr char label[] = "crap_tube";
	static constexpr char name[] = "crap Tube Distortion";
	static constexpr char author[] = "Connor Olding";
	static constexpr char copyright[] = "MIT";

	static constexpr ulong parameters = 2;

	smoothval drive, wet;

	inline void
	process2(v2df *buf, ulong rem)
	{
		v2df drives[FULL_SIZE], wets[FULL_SIZE];
		for (ulong i = 0; i < rem; i++)
			drives[i] = v2df(drive.smooth());
		for (ulong i = 0; i < rem; i++)
			drives[i] = v2df(wet.smooth());

		for (ulong i = 0; i < rem; i++)
			buf[i] = Tube::process(buf[i], drives[i], wets[i]);
	}

	inline void
	resume()
	{
		memset(&hb_up,   0, sizeof(halfband_t<v2df>));
		memset(&hb_down, 0, sizeof(halfband_t<v2df>));
	}

	inline void
	pause()
	{}

	static inline void
	construct_params(Param *params)
	{
		sprintf(params[0].name, "Drive");
		params[0].min = -30;
		params[0].max = 15;
		params[0].scale = SCALE_DB;
		params[0].def = DEFAULT_0;
		params[0].reset();

		sprintf(params[1].name, "Wetness");
		params[1].min = 0;
		params[1].max = 1;
		params[1].scale = SCALE_FLOAT;
		params[1].def = DEFAULT_1;
		params[1].reset();
	}

	inline void
	adjust(Param *params, ulong fs_long)
	{
		double fs = fs_long;
		drive.desired = DB2LIN(params[0].value);
		wet.desired = params[1].value;
		drive.actual = drive.desired;
		wet.actual = wet.desired;
		drive.speed = 1 + 20/fs/OVERSAMPLING;
		wet.speed = 20/fs/OVERSAMPLING;
		drive.log = 1;
		wet.log = 0;
	}

	inline void
	adjust_one(Param *params, int index)
	{
		drive.desired = DB2LIN(params[0].value);
		wet.desired = params[1].value;
	}
};
