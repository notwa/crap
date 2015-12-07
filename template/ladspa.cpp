//#INCLUDE
//#REDEFINE

#include <stdlib.h>
#include <stdio.h>
#ifdef _MSC_VER
// we need memcpy
#include <memory.h>
#endif

#include "ladspa.hpp"

#ifndef PARAM_NAME_LEN
#define PARAM_NAME_LEN 25
#endif

enum {
	PLUG_INPUT_L,
	PLUG_INPUT_R,
	PLUG_OUTPUT_L,
	PLUG_OUTPUT_R,
	IO_PLUGS
};

#define ALLOC(type, amount) (type *) calloc(amount, sizeof(type))

char p_default_strings[IO_PLUGS][PARAM_NAME_LEN] = {
	"Input L", "Input R",
	"Output L", "Output R"
};

int
param2hint(Param *p)
{
	int hint = LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
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

	return hint;
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
struct LADSPA_Plugin {
	//static const ulong name_buf_size = (portcount)*PARAM_NAME_LEN;
	static const ulong portcount = IO_PLUGS + T::parameters;
	static Param *default_params;
	static LADSPA_PortDescriptor descs[portcount];
	static LADSPA_PortRangeHint hints[portcount];
	static char* names[portcount];
	static char name_buffer[portcount][PARAM_NAME_LEN];

	static void
	init()
	{
		for (int i = 0; i < portcount; i++)
			names[i] = name_buffer[i];

		for (int i = 0; i < IO_PLUGS; i++) {
			memcpy(names[i], p_default_strings[i], PARAM_NAME_LEN);
			descs[i] = LADSPA_PORT_AUDIO;
			descs[i] |= (i < 2) ? LADSPA_PORT_INPUT : LADSPA_PORT_OUTPUT;
			hints[i] = {0, 0, 0};
		}

		default_params = NULL;
		if (T::parameters)
			default_params = new Param[T::parameters];
		T::construct_params(default_params);
		for (int i = 0; i < T::parameters; i++) {
			int j = i + IO_PLUGS;
			Param *p = &default_params[i];

			memcpy(names[j], p->name, PARAM_NAME_LEN);
			descs[j] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;

			hints[j].LowerBound = p->min;
			hints[j].UpperBound = p->max;

			hints[j].HintDescriptor = param2hint(p);
		}
	};

	static LADSPA_Handle
	plug_construct(const LADSPA_Descriptor *descriptor, unsigned long fs)
	{
		plug_t *plug = ALLOC(plug_t, 1);
		plug->crap = new T();
		if (T::parameters > 0) {
			plug->values = ALLOC(LADSPA_Data *, T::parameters);
			plug->params = ALLOC(Param, T::parameters);
			memcpy(plug->params, default_params, sizeof(Param)*T::parameters);
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
		else if (T::parameters > 0 && port < T::parameters + IO_PLUGS)
			plug->values[port - IO_PLUGS] = data;
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

#define P LADSPA_Plugin<T>
TEMPLATE Param *P::default_params;
TEMPLATE LADSPA_PortDescriptor P::descs[P::portcount];
TEMPLATE LADSPA_PortRangeHint P::hints[P::portcount];
TEMPLATE char* P::names[P::portcount];
TEMPLATE char P::name_buffer[P::portcount][PARAM_NAME_LEN];

TEMPLATE static
LADSPA_Descriptor gen_desc() {
	P::init();
	LADSPA_Descriptor d = {};

	d.UniqueID = T::id;
	d.Label = T::label;
	d.Properties = 0;
	d.Name = T::name;
	d.Maker = T::author;
	d.Copyright = T::copyright;
	d.PortCount = P::portcount;
	d.PortDescriptors = P::descs;
	d.PortRangeHints = P::hints;
	d.PortNames = (const char *const *) P::names;

	d.instantiate = P::plug_construct;
	d.cleanup = P::plug_destruct;
	d.activate = P::plug_resume;
	d.deactivate = P::plug_pause;
	d.connect_port = P::plug_connect;
	d.run = P::plug_process;
	d.run_adding = NULL;
	d.set_run_adding_gain = NULL;

	return d;
}
#undef P

static LADSPA_Descriptor plug_descs[] = {
	gen_desc<CrapPlug>()
};

const LADSPA_Descriptor *
ladspa_descriptor(unsigned long index)
{
	if (index >= sizeof(plug_descs)/sizeof(plug_descs[0]))
		return NULL;
	return plug_descs + index;
}
