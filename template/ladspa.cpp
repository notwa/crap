#include <stdlib.h>
#include "ladspa.hpp"

//#INCLUDE
//#REDEFINE

#ifndef PARAM_NAME_LEN
#define PARAM_NAME_LEN 24
#endif

#define PLUG_INPUT_L 0
#define PLUG_INPUT_R 1
#define PLUG_OUTPUT_L 2
#define PLUG_OUTPUT_R 3
#define PCOUNT (PARAMETERS + 4)

// LADSPA is a jerk and won't let us initialize anything
// before asking for a descriptor.
// in reality we could use a crap-ton of constexprs,
// but i just need something functional for now
static void __attribute__ ((constructor)) plug_init();
static void __attribute__ ((destructor)) plug_cleanup();

#define ALLOC(type, amount) (type *) calloc(amount, sizeof(type))

char p_default_strings[4][PARAM_NAME_LEN + 1] = {
	"Input L", "Input R",
	"Output L", "Output R"
};

LADSPA_PortDescriptor p_descs[PCOUNT];
LADSPA_PortRangeHint p_hints[PCOUNT];
static Param *global_params;
char **p_name_strings;
char *p_names[PCOUNT];

static void
plug_cleanup()
{
	for (int i = 0; i < PARAMETERS; i++) {
		free(p_name_strings[i]);
	}
	free(p_name_strings);
	free(global_params);
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

	global_params = ALLOC(Param, PARAMETERS);
	p_name_strings = ALLOC(char *, PARAMETERS);

	CrapPlug::construct_params(global_params);
	for (int i = 0; i < PARAMETERS; i++) {
		p_name_strings[i] = ALLOC(char, PARAM_NAME_LEN + 1);

		int j = i + 4;
		Param *p = &global_params[i];

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
}

struct plug_t {
	LADSPA_Data *input_L;
	LADSPA_Data *input_R;
	LADSPA_Data *output_L;
	LADSPA_Data *output_R;

	Crap *crap;
	LADSPA_Data **values;
	Param *params;
};

TEMPLATE
struct LADSPA_Plugin : public T {
	static LADSPA_Handle
	plug_construct(const LADSPA_Descriptor *descriptor, unsigned long fs)
	{
		plug_t *plug = ALLOC(plug_t, 1);
		plug->crap = new CrapPlug();
		if (T::parameters > 0) {
			plug->values = ALLOC(LADSPA_Data *, T::parameters);
			plug->params = ALLOC(Param, T::parameters);
			memcpy(plug->params, global_params, sizeof(Param)*T::parameters);
			plug->crap->adjust(plug->params, fs);
		} else {
			plug->crap->adjust(NULL, fs);
		}
		return (LADSPA_Handle) plug;
	}

	static void
	plug_destruct(LADSPA_Handle instance)
	{
		plug_t *plug = (plug_t *)instance;
		delete plug->crap;
		free(plug->values);
		free(plug->params);
		free(plug);
	}

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
		else if (T::parameters > 0 && port < T::parameters + 4)
			plug->values[port - 4] = data;
	}

	static void
	plug_resume(LADSPA_Handle instance)
	{
		plug_t *plug = (plug_t *)instance;
		plug->crap->resume();
	}

	static void
	plug_pause(LADSPA_Handle instance)
	{
		plug_t *plug = (plug_t *)instance;
		plug->crap->pause();
	}

	static void
	plug_process(LADSPA_Handle instance, unsigned long count)
	{
		plug_t *plug = (plug_t *)instance;
		for (int i = 0; i < T::parameters; i++) {
			if (!plug->values[i])
				continue;
			if (*plug->values[i] != plug->params[i].value) {
				plug->params[i].value = *plug->values[i];
				plug->crap->adjust_one(plug->params, i);
			}
		}
		plug->crap->process(
		    plug->input_L, plug->input_R,
		    plug->output_L, plug->output_R,
		    count);
	}
};

TEMPLATE static constexpr
LADSPA_Descriptor gen_desc() {
	return LADSPA_Descriptor {
		.UniqueID = T::id,
		.Label = T::label,
		.Properties = 0,
		.Name = T::name,
		.Maker = T::author,
		.Copyright = T::copyright,
		.PortCount = 4 + T::parameters,
		.PortDescriptors = p_descs,
		.PortRangeHints = p_hints,
		.PortNames = (const char * const *) p_names,

		.instantiate = T::plug_construct,
		.cleanup = T::plug_destruct,
		.activate = T::plug_resume,
		.deactivate = T::plug_pause,
		.connect_port = T::plug_connect,
		.run = T::plug_process,
		.run_adding = NULL,
		.set_run_adding_gain = NULL
	};
}

static constexpr LADSPA_Descriptor plug_descs[] = {
	gen_desc<LADSPA_Plugin<CrapPlug>>()
};

const LADSPA_Descriptor *
ladspa_descriptor(unsigned long index)
{
	if (index >= sizeof(plug_descs)/sizeof(plug_descs[0]))
		return NULL;
	return plug_descs + index;
}
