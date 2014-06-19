#include <stdlib.h>
#include "ladspa.h"

//#INCLUDE

#ifndef PARAM_NAME_LEN
#define PARAM_NAME_LEN 24
#endif

#define PLUG_INPUT_L 0
#define PLUG_INPUT_R 1
#define PLUG_OUTPUT_L 2
#define PLUG_OUTPUT_R 3
#define PCOUNT (PARAMETERS + 4)

static void __attribute__ ((constructor)) plug_init();

LADSPA_PortDescriptor p_descs[PCOUNT];
LADSPA_PortRangeHint p_hints[PCOUNT];
char p_default_strings[4][PARAM_NAME_LEN + 1] = {
	"Input L", "Input R",
	"Output L", "Output R"
};
#if (PARAMETERS > 0)
static param global_params[PARAMETERS];
char p_name_strings[PARAMETERS][PARAM_NAME_LEN + 1];
#endif
char *p_names[PCOUNT];

typedef struct {
	LADSPA_Data *input_L;
	LADSPA_Data *input_R;
	LADSPA_Data *output_L;
	LADSPA_Data *output_R;

	personal data;
	#if (PARAMETERS > 0)
	LADSPA_Data *values[PARAMETERS];
	param params[PARAMETERS];
	#endif
} plug_t;

static void
plug_connect(LADSPA_Handle instance, unsigned long port, LADSPA_Data *data)
{
	plug_t *plug = (plug_t *)instance;
	if (port == PLUG_INPUT_L)
		plug->input_L = data;
	else if (port == PLUG_INPUT_R)
		plug->input_R = data;
	else if (port == PLUG_OUTPUT_L)
		plug->output_L = data;
	else if (port == PLUG_OUTPUT_R)
		plug->output_R = data;
	#if (PARAMETERS > 0)
	else if (port < PARAMETERS + 4)
		plug->values[port - 4] = data;
	#endif
}

static void
plug_resume(LADSPA_Handle instance)
{
	plug_t *plug = (plug_t *)instance;
	resume(&plug->data);
}

static void
plug_pause(LADSPA_Handle instance)
{
	plug_t *plug = (plug_t *)instance;
	pause(&plug->data);
}

static LADSPA_Handle
plug_construct(const LADSPA_Descriptor *descriptor, unsigned long fs)
{
	plug_t *plug = (plug_t *) calloc(1, sizeof(plug_t));
	construct(&plug->data);
	#if (PARAMETERS > 0)
	memcpy(plug->params, global_params, sizeof(param)*PARAMETERS);
	adjust(&plug->data, plug->params, fs);
	#else
	adjust(&plug->data, fs);
	#endif
	return (LADSPA_Handle) plug;
}

static void
plug_destruct(LADSPA_Handle instance)
{
	plug_t *plug = (plug_t *)instance;
	destruct(&plug->data);
	free(plug);
}

static void
plug_process(LADSPA_Handle instance, unsigned long count)
{
	plug_t *plug = (plug_t *)instance;
	#if (PARAMETERS > 0)
	for (int i = 0; i < PARAMETERS; i++) {
		if (!plug->values[i])
			continue;
		if (*plug->values[i] != plug->params[i].value) {
			plug->params[i].value = *plug->values[i];
			adjust_one(&plug->data, plug->params, i);
		}
	}
	#endif
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
	.PortDescriptors = p_descs,
	.PortRangeHints = p_hints,
	.PortNames = (const char * const *) p_names,

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
ladspa_descriptor(unsigned long index)
{
	if (index != 0)
		return NULL;
	return &plug_desc;
}

static void
plug_init()
{
	for (int i = 0; i < 4; i++) {
		p_names[i] = p_default_strings[i];
		p_descs[i] = LADSPA_PORT_AUDIO;
		p_descs[i] |= (i < 2) ? LADSPA_PORT_INPUT : LADSPA_PORT_OUTPUT;
		p_hints[i] = (LADSPA_PortRangeHint){.HintDescriptor = 0};
	}

	#if (PARAMETERS > 0)
	construct_params(global_params);
	for (int i = 0; i < PARAMETERS; i++) {
		int j = i + 4;
		param *p = &global_params[i];

		memcpy(p_name_strings[i], p->name, PARAM_NAME_LEN + 1);
		p_names[j] = p_name_strings[i];
		p_descs[j] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		int hint = LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;

		p_hints[j].LowerBound = p->min;
		p_hints[j].UpperBound = p->max;

		if (p->scale == SCALE_INT)    hint |= LADSPA_HINT_INTEGER;
		if (p->scale == SCALE_TOGGLE) hint |= LADSPA_HINT_TOGGLED;
		if (p->scale >= SCALE_LOG)    hint |= LADSPA_HINT_LOGARITHMIC;

		switch (p->def) {
		case DEFAULT_0:    hint |= LADSPA_HINT_DEFAULT_0;       break;
		case DEFAULT_1:    hint |= LADSPA_HINT_DEFAULT_1;       break;
		case DEFAULT_100:  hint |= LADSPA_HINT_DEFAULT_100;     break;
		case DEFAULT_440:  hint |= LADSPA_HINT_DEFAULT_440;     break;
		case DEFAULT_MIN:  hint |= LADSPA_HINT_DEFAULT_MINIMUM; break;
		case DEFAULT_LOW:  hint |= LADSPA_HINT_DEFAULT_LOW;     break;
		case DEFAULT_HIGH: hint |= LADSPA_HINT_DEFAULT_HIGH;    break;
		case DEFAULT_MAX:  hint |= LADSPA_HINT_DEFAULT_MAXIMUM; break;
		case DEFAULT_HALF: hint |= LADSPA_HINT_DEFAULT_MIDDLE;  break;
		}

		p_hints[j].HintDescriptor = hint;
	}
	#endif
}
