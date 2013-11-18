#include <stdlib.h>
#include "ladspa.h"

//#INCLUDE

#define PLUG_INPUT_L 0
#define PLUG_INPUT_R 1
#define PLUG_OUTPUT_L 2
#define PLUG_OUTPUT_R 3
#define PCOUNT (PARAMETERS + 4)

const LADSPA_PortDescriptor p_discs[PCOUNT] = {
	LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO
	,
	LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO
	,
	LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO
	,
	LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO
//#PORT_DESCRIPTIONS
};

const LADSPA_PortRangeHint p_hints[PCOUNT] = {
	{.HintDescriptor = 0}
	,
	{.HintDescriptor = 0}
	,
	{.HintDescriptor = 0}
	,
	{.HintDescriptor = 0}
//#PORT_HINTS
};

const char *p_names[PCOUNT] = {
	"L input"
	,
	"R input"
	,
	"L output"
	,
	"R output"
//#PORT_NAMES
};

typedef struct {
	LADSPA_Data *input_L;
	LADSPA_Data *input_R;
	LADSPA_Data *output_L;
	LADSPA_Data *output_R;

//#PORT_DATA
	personal data;
} plug_t;

static void
plug_connect(LADSPA_Handle instance, unsigned long port, LADSPA_Data *data) {
	plug_t *plug = (plug_t *)instance;
	if (port == PLUG_INPUT_L)
		plug->input_L = data;
	else if (port == PLUG_INPUT_R)
		plug->input_R = data;
	else if (port == PLUG_OUTPUT_L)
		plug->output_L = data;
	else if (port == PLUG_OUTPUT_R)
		plug->output_R = data;
//#PORT_CONNECT
}

static void
plug_resume(LADSPA_Handle instance) {
	plug_t *plug = (plug_t *)instance;
	resume(&plug->data);
}

static void
plug_pause(LADSPA_Handle instance) {
	plug_t *plug = (plug_t *)instance;
	pause(&plug->data);
}

static LADSPA_Handle
plug_construct(const LADSPA_Descriptor *descriptor, unsigned long fs) {
	plug_t *plug = (plug_t *) calloc(1, sizeof(plug_t));
	construct(&plug->data);
	adjust(&plug->data, fs);
	return (LADSPA_Handle) plug;
}

static void
plug_destruct(LADSPA_Handle instance) {
	plug_t *plug = (plug_t *)instance;
	destruct(&plug->data);
	free(plug);
}

static void
plug_process(LADSPA_Handle instance, unsigned long count) {
	plug_t *plug = (plug_t *)instance;
	process(&plug->data,
	    plug->input_L, plug->input_R,
	    plug->output_L, plug->output_R,
	    count);
}

static const LADSPA_Descriptor plug_desc = {
	.UniqueID = ID,
	.Label = LABEL,
	.Properties = 0,
	.Name = NAME,
	.Maker = AUTHOR,
	.Copyright = COPYRIGHT,
	.PortCount = PCOUNT,
	.PortDescriptors = p_discs,
	.PortRangeHints = p_hints,
	.PortNames = p_names,

	.instantiate = plug_construct,
	.cleanup = plug_destruct,
	.activate = plug_resume,
	.deactivate = plug_pause,
	.connect_port = plug_connect,
	.run = plug_process,
	.run_adding = NULL,
	.set_run_adding_gain = NULL
};

const LADSPA_Descriptor *
ladspa_descriptor(unsigned long index) {
	if (index != 0)
		return NULL;
	return &plug_desc;
}
