#include <stdlib.h>
#include <math.h>

#include "ladspa.h"
#include "crap_util.h"

typedef unsigned long ulong;

#define REALBANDS 4
#define EQ_INPUT  0
#define EQ_OUTPUT 1
#define PCOUNT    2

const LADSPA_PortDescriptor p_discs[PCOUNT] = {
    LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO,
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO
};
const LADSPA_PortRangeHint p_hints[PCOUNT] = {
    {.HintDescriptor = 0},
    {.HintDescriptor = 0}
};
const char *p_names[PCOUNT] = {"Input", "Output"};

typedef struct {
	LADSPA_Data *input;
	LADSPA_Data *output;

	biquad filters[REALBANDS];
	LADSPA_Data fs;

	LADSPA_Data run_adding_gain;
} eq_t;

static void
activate_eq(LADSPA_Handle instance) {
	eq_t *eq = (eq_t *)instance;
	biquad *filters = eq->filters;

	for (int i = 0; i < REALBANDS; i++)
		biquad_init(&filters[i]);
}

static void
cleanup_eq(LADSPA_Handle instance) {
	free(instance);
}

static void
connect_port_eq(LADSPA_Handle instance, ulong port, LADSPA_Data *data) {
	eq_t *eq = (eq_t *)instance;
	if (port == EQ_INPUT)
		eq->input = data;
	else if (port == EQ_OUTPUT)
		eq->output = data;
}

static LADSPA_Handle
instantiate_eq(const LADSPA_Descriptor *descriptor, ulong s_rate) {
	eq_t *eq = (eq_t *) calloc(1, sizeof(eq_t));
	biquad *filters = eq->filters;
	LADSPA_Data fs = s_rate;

	eq->fs = fs;
	eq->run_adding_gain = 1;

	filters[0] = biquad_gen(4,   10,  0.0, 1.00, fs);
	filters[1] = biquad_gen(0,   36,  4.3, 1.25, fs);
	filters[2] = biquad_gen(2, 1400, -7.0, 1.20, fs);
	filters[3] = biquad_gen(0, 7000,  3.3, 1.25, fs);

	return (LADSPA_Handle) eq;
}

static void
run_eq_for_real(LADSPA_Handle instance, ulong sample_count, const int running) {
	eq_t *eq = (eq_t *) instance;

	biquad *filters = eq->filters;

	const LADSPA_Data *input = eq->input;
	LADSPA_Data *output = eq->output;

	for (ulong pos = 0; pos < sample_count; pos++) {
		LADSPA_Data samp = input[pos];
		for (int i = 0; i < REALBANDS; i++)
			samp = biquad_run(&filters[i], samp);
		if (running)
			output[pos] += eq->run_adding_gain * samp;
		else
			output[pos] = samp;
	}
}

static void
run_eq(LADSPA_Handle instance, ulong sample_count) {
	run_eq_for_real(instance, sample_count, 0);
}

static void
run_adding_eq(LADSPA_Handle instance, ulong sample_count) {
	run_eq_for_real(instance, sample_count, 1);
}

void
set_run_adding_gain(LADSPA_Handle instance, LADSPA_Data gain) {
	eq_t *eq = (eq_t *) instance;
	eq->run_adding_gain = gain;
}

static const LADSPA_Descriptor eqDescriptor = {
	.UniqueID = 0xDEFACED,
	.Label = "crap_eq_const",
	.Properties = 0,
	.Name = "crap const Equalizer",
	.Maker = "Connor Olding",
	.Copyright = "MIT",
	.PortCount = PCOUNT,
	.PortDescriptors = p_discs,
	.PortRangeHints = p_hints,
	.PortNames = p_names,

	.activate = activate_eq,
	.cleanup = cleanup_eq,
	.connect_port = connect_port_eq,
	.deactivate = NULL,
	.instantiate = instantiate_eq,
	.run = run_eq,
	.run_adding = run_adding_eq,
	.set_run_adding_gain = set_run_adding_gain
};

const LADSPA_Descriptor *
ladspa_descriptor(ulong index) {
	if (index != 0)
		return NULL;
	return &eqDescriptor;
}
