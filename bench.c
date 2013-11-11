#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "dlfcn.h"
#include "ladspa.h"
#include "crap_util.h"

enum {
	BLOCK_SIZE=2048
};

float inputs[BLOCK_SIZE];
float outputs[BLOCK_SIZE];
void *plug = NULL;

static void
cleanup() {
	dlclose(plug);
}

static const LADSPA_Descriptor*
load_ladspa(char *path) {
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
main(int argc, char **argv) {
	assert(argc > 1);

	const LADSPA_Descriptor *d = load_ladspa(argv[1]);

	LADSPA_Handle h = d->instantiate(d, 44100);
	assert(h);

	d->connect_port(h, 0, inputs);
	d->connect_port(h, 1, outputs);

	mirand = time(NULL);
	for (int i = 0; i < BLOCK_SIZE; i++)
		inputs[i] = whitenoise();

	if (d->activate) d->activate(h);
	for (int i = 0; i < 64*64*8; i++)
		d->run(h, BLOCK_SIZE);
	if (d->deactivate) d->deactivate(h);

	d->cleanup(h);

	return 0;
}
