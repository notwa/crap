#include "util.h"

#define ID 0xEC57A71C
#define LABEL "crap_noise"
#define NAME "crap noise generator"
#define AUTHOR "Connor Olding"
#define COPYRIGHT "MIT"
#define PARAMETERS 0

typedef struct {
} personal;

INNER void
process(personal *data,
    float *in_L, float *in_R,
    float *out_L, float *out_R,
    unsigned long count)
{
	// TODO: separate and preserve mirand for each channel
	for (unsigned long pos = 0; pos < count; pos++)
		out_L[pos] = whitenoise();
	for (unsigned long pos = 0; pos < count; pos++)
		out_R[pos] = whitenoise();
}

INNER void
process_double(personal *data,
    double *in_L, double *in_R,
    double *out_L, double *out_R,
    unsigned long count)
{
	for (unsigned long pos = 0; pos < count; pos++)
		out_L[pos] = whitenoise();
	for (unsigned long pos = 0; pos < count; pos++)
		out_R[pos] = whitenoise();
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
adjust(personal *data, unsigned long fs)
{}
