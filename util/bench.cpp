#include <alloca.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <math.h>

#include "dlfcn.h"
#include "ladspa.hpp"
#include "util.hpp"

#define BENCH_BLOCK 2048

void *plug = NULL;
static float *audio_buffer;
static int audio_count = 0;

INNER void
cleanup()
{
	dlclose(plug);
	if (audio_count) free(audio_buffer);
}

INNER const LADSPA_Descriptor*
load_ladspa(char *path)
{
	plug = dlopen(path, RTLD_NOW);
	if (!plug) {
		puts(dlerror());
		exit(1);
	}
	atexit(cleanup);

	LADSPA_Descriptor_Function df;
	df = (decltype(df)) dlsym(plug, "ladspa_descriptor");
	if (!df) {
		puts(dlerror());
		exit(1);
	}

	const LADSPA_Descriptor *d = df(0);
	assert(d);

	return d;
}

INNER float
between(float percent, float min, float max, int logscale)
{
	if (logscale)
		return log(min/percent)/log(min/max);
	else
		return (min - percent)/(min - max);
}

INNER float
get_default(LADSPA_PortRangeHint hint)
{
	float x = 0;
	int hd = hint.HintDescriptor;
	float min = hint.LowerBound;
	float max = hint.UpperBound;
	float logscale = LADSPA_IS_HINT_LOGARITHMIC(hd);
	if (LADSPA_IS_HINT_DEFAULT_0(hd))
		x = 0;
	if (LADSPA_IS_HINT_DEFAULT_1(hd))
		x = 1;
	if (LADSPA_IS_HINT_DEFAULT_100(hd))
		x = 100;
	if (LADSPA_IS_HINT_DEFAULT_440(hd))
		x = 440;
	if (LADSPA_IS_HINT_DEFAULT_MINIMUM(hd))
		x = min;
	if (LADSPA_IS_HINT_DEFAULT_LOW(hd))
		x = between(0.25, min, max, logscale);
	if (LADSPA_IS_HINT_DEFAULT_MIDDLE(hd))
		x = between(0.50, min, max, logscale);
	if (LADSPA_IS_HINT_DEFAULT_HIGH(hd))
		x = between(0.75, min, max, logscale);
	if (LADSPA_IS_HINT_DEFAULT_MAXIMUM(hd))
		x = max;
	if (LADSPA_IS_HINT_INTEGER(hd))
		x = round(x);
	if (LADSPA_IS_HINT_TOGGLED(hd)) {
		float mid = between(0.50, min, max, logscale);
		x = x >= mid ? max : min;
	}
	if (x < min) x = min;
	if (x > max) x = max;
	return x;
}

int
main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "Please supply a path to the plugin to test.\n");
		return 1;
	}

	const LADSPA_Descriptor *d = load_ladspa(argv[1]);

	LADSPA_Handle h = d->instantiate(d, 44100);
	assert(h);

	// we're lazy so we don't distinguish inputs and outputs
	for (int i = 0; i < d->PortCount; i++)
		if (LADSPA_IS_PORT_AUDIO(d->PortDescriptors[i]))
			audio_count++;

	audio_buffer = (decltype(audio_buffer)) calloc(audio_count*BENCH_BLOCK, sizeof(float));

	int a = 0;
	for (int i = 0; i < d->PortCount; i++) {
		if (LADSPA_IS_PORT_AUDIO(d->PortDescriptors[i])) {
			d->connect_port(h, i, audio_buffer + a++*BENCH_BLOCK);
		} else {
			float *x;
			x = (decltype(x)) alloca(sizeof(float));
			*x = get_default(d->PortRangeHints[i]);
			d->connect_port(h, i, x);
		}
	}

	unsigned int mirand = time(NULL);
	for (int i = 0; i < audio_count*BENCH_BLOCK; i++)
		audio_buffer[i] = whitenoise(mirand);

	if (d->activate) d->activate(h);
	for (int i = 0; i < 64*64*8; i++)
		d->run(h, BENCH_BLOCK);
	if (d->deactivate) d->deactivate(h);

	d->cleanup(h);

	return 0;
}
