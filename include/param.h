#define PARAM_NAME_LEN 24

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

typedef struct {
	char name[PARAM_NAME_LEN + 1];
	float value, min, max;
	param_scale scale;
	param_default def;
} param;

#include <math.h>

void
param_set(param *p, float percent)
{
	if (p->scale >= SCALE_LOG)
		p->value = exp(percent*(log(p->max/p->min)))*p->min;
	else
		p->value = percent*(p->max - p->min) + p->min;
}

float
param_get(param *p)
{
	if (p->scale >= SCALE_LOG)
		return log(p->min/p->value)/log(p->min/p->max);
	else
		return (p->min - p->value)/(p->min - p->max);
}

void
param_reset(param *p)
{
	switch (p->def) {
	case DEFAULT_0:         p->value = 0;           break;
	case DEFAULT_1:         p->value = 1;           break;
	case DEFAULT_100:       p->value = 100;         break;
	case DEFAULT_440:       p->value = 440;         break;
	case DEFAULT_MIN:       p->value = p->min;      break;
	case DEFAULT_LOW:       param_set(p, 0.25);     break;
	case DEFAULT_HALF:      param_set(p, 0.5);      break;
	case DEFAULT_HIGH:      param_set(p, 0.75);     break;
	case DEFAULT_MAX:       p->value = p->max;      break;
	}
}
