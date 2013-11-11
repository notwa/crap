#include "ladspa.h"
#include "crap_util.h"

typedef unsigned long ulong;

#define PLUG_INPUT  0
#define PLUG_OUTPUT 1
#define PCOUNT      2

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
	LADSPA_Data fs;
} plug_t;

static void
plug_cleanup(LADSPA_Handle instance) {
	free(instance);
}

static void
plug_connect(LADSPA_Handle instance, ulong port, LADSPA_Data *data) {
	plug_t *plug = (plug_t *)instance;
	if (port == PLUG_INPUT)
		plug->input = data;
	else if (port == PLUG_OUTPUT)
		plug->output = data;
}

static LADSPA_Handle
plug_instantiate(const LADSPA_Descriptor *descriptor, ulong s_rate) {
	plug_t *plug = (plug_t *) calloc(1, sizeof(plug_t));
	plug->fs = s_rate;
	return (LADSPA_Handle) plug;
}

static void
plug_run(LADSPA_Handle instance, ulong sample_count) {
	plug_t *plug = (plug_t *) instance;
	//const LADSPA_Data *input = plug->input;
	LADSPA_Data *output = plug->output;

	for (ulong pos = 0; pos < sample_count; pos++)
		output[pos] = whitenoise();
}

static const LADSPA_Descriptor eqDescriptor = {
	.UniqueID = 0xEC57A71C,
	.Label = "crap_noise",
	.Properties = 0,
	.Name = "crap noise generator",
	.Maker = "Connor Olding",
	.Copyright = "MIT",
	.PortCount = PCOUNT,
	.PortDescriptors = p_discs,
	.PortRangeHints = p_hints,
	.PortNames = p_names,

	.activate = NULL,
	.cleanup = plug_cleanup,
	.connect_port = plug_connect,
	.deactivate = NULL,
	.instantiate = plug_instantiate,
	.run = plug_run,
	.run_adding = NULL,
	.set_run_adding_gain = NULL
};

const LADSPA_Descriptor *
ladspa_descriptor(ulong index) {
	if (index != 0)
		return NULL;
	return &eqDescriptor;
}
