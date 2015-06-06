#include "util.hpp"

#define ID 0xEC57A71C
#define LABEL "crap_noise"
#define NAME "crap noise generator"
#define AUTHOR "Connor Olding"
#define COPYRIGHT "MIT"
#define PARAMETERS 0

typedef struct {
} personal;

template<typename T>
INNER void
process(personal *data,
    T *in_L, T *in_R,
    T *out_L, T *out_R,
    unsigned long count)
{
	// TODO: separate and preserve mirand for each channel
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
