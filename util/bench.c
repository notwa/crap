#include <alloca.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "dlfcn.h"
#include "ladspa.h"
#include "util.h"

enum {
	BLOCK_SIZE=2048
};

void *plug = NULL;
static float *audio_buffer;
static int audio_count = 0;

static void
cleanup()
{
	dlclose(plug);
	if (audio_count) free(audio_buffer);
}

static const LADSPA_Descriptor*
load_ladspa(char *path)
{
	plug = dlopen(path, RTLD_NOW);
	assert(plug);
	atexit(cleanup);

	LADSPA_Descriptor_Function df = dlsym(plug, "ladspa_descriptor");
	assert(df);

	const LADSPA_Descriptor *d = df(0);
	assert(d);

	return d;
}

int
main(int argc, char **argv)
{
	assert(argc > 1);

	const LADSPA_Descriptor *d = load_ladspa(argv[1]);

	LADSPA_Handle h = d->instantiate(d, 44100);
	assert(h);

	// we're lazy so we don't distinguish inputs and outputs
	for (int i = 0; i < d->PortCount; i++)
		if (LADSPA_IS_PORT_AUDIO(d->PortDescriptors[i]))
			audio_count++;

	audio_buffer = calloc(audio_count*BLOCK_SIZE, sizeof(float));

	int a = 0;
	for (int i = 0; i < d->PortCount; i++) {
		if (LADSPA_IS_PORT_AUDIO(d->PortDescriptors[i])) {
			d->connect_port(h, i, audio_buffer + a++*BLOCK_SIZE);
		} else {
			float *x = alloca(sizeof(float));
			*x = 0;
			d->connect_port(h, i, x);
		}
	}

	mirand = time(NULL);
	for (int i = 0; i < audio_count*BLOCK_SIZE; i++)
		audio_buffer[i] = whitenoise();

	if (d->activate) d->activate(h);
	for (int i = 0; i < 64*64*8; i++)
		d->run(h, BLOCK_SIZE);
	if (d->deactivate) d->deactivate(h);

	d->cleanup(h);

	return 0;
}
