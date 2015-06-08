// including null-terminator!
#define PARAM_NAME_LEN 25

// TOOD: dump enums in param namespace

typedef enum {
	SCALE_FLOAT,
	SCALE_INT,
	SCALE_TOGGLE,
	SCALE_DB,
	SCALE_LOG,
	SCALE_HZ
} param_scale;

typedef enum {
	DEFAULT_0,
	DEFAULT_1,
	DEFAULT_100,
	DEFAULT_440,
	DEFAULT_MIN,
	DEFAULT_LOW,  // 25%
	DEFAULT_HALF, // 50%
	DEFAULT_HIGH, // 75%
	DEFAULT_MAX
} param_default;

struct Param {
	char name[PARAM_NAME_LEN];
	float value, min, max;
	param_scale scale;
	param_default def;

	Param()
	{}

	/*
	Param(
	    char *name, float min, float max,
	    param_scale scale, param_default def)
	: name(name), min(min), max(max), scale(scale), def(def)
	{
		reset()
	}
	*/

	void
	set(float percent)
	{
		if (scale >= SCALE_LOG)
			value = exp(percent*(log(max/min)))*min;
		else
			value = percent*(max - min) + min;
	}

	float
	get()
	{
		if (scale >= SCALE_LOG)
			return log(min/value)/log(min/max);
		else
			return (min - value)/(min - max);
	}

	void
	reset()
	{
		switch (def) {
		case DEFAULT_0:         value = 0;       break;
		case DEFAULT_1:         value = 1;       break;
		case DEFAULT_100:       value = 100;     break;
		case DEFAULT_440:       value = 440;     break;
		case DEFAULT_MIN:       value = min;     break;
		case DEFAULT_LOW:       set(0.25);       break;
		case DEFAULT_HALF:      set(0.5);        break;
		case DEFAULT_HIGH:      set(0.75);       break;
		case DEFAULT_MAX:       value = max;     break;
		}
	}
};
