#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#include "ladspa.h"
#include "crap_util.h"

typedef unsigned long ulong;

/* Ports:
 * BANDS*0 to BANDS*1-1: gain
 * BANDS*1 to BANDS*2-1: frequency
 * BANDS*2 to BANDS*3-1: bandwidth
 */
#ifndef BANDS
#define BANDS 4
#endif
#define EQ_INPUT  (BANDS*3+0)
#define EQ_OUTPUT (BANDS*3+1)
#define PCOUNT    (BANDS*3+2)

#define GAIN_MIN -60
#define GAIN_MAX 18
#define FREQ_MIN 1
#define FREQ_MAX 20000
#define BW_MIN 0.02
#define BW_MAX 8

static int ready = 0;
void _init();

LADSPA_PortDescriptor p_discs[PCOUNT];
LADSPA_PortRangeHint p_hints[PCOUNT];
const char *p_names[PCOUNT];

static LADSPA_Descriptor eqDescriptor = {
	.UniqueID = 0xCAFED,
	.Label = "crap_eq",
	.Properties = 0,
	.Name = "crap Parametric Equalizer",
	.Maker = "Connor Olding",
	.Copyright = "MIT",
	.PortCount = PCOUNT,
	.PortDescriptors = p_discs,
	.PortRangeHints = p_hints,
	.PortNames = p_names
};

typedef struct {
	LADSPA_Data *chg[BANDS];
	LADSPA_Data *chf[BANDS];
	LADSPA_Data *chb[BANDS];
	LADSPA_Data old_chg[BANDS];
	LADSPA_Data old_chf[BANDS];
	LADSPA_Data old_chb[BANDS];
	LADSPA_Data *input;
	LADSPA_Data *output;

	biquad filters[BANDS];
	LADSPA_Data fs;

	LADSPA_Data run_adding_gain;
} eq_t;

DLL_EXPORT const LADSPA_Descriptor *
ladspa_descriptor(ulong index) {
	if (!ready) _init();

	if (index != 0)
		return NULL;
	return &eqDescriptor;
}

static void
activate_eq(LADSPA_Handle instance) {
	eq_t *eq = (eq_t *)instance;
	biquad *filters = eq->filters;

	for (int i = 0; i < BANDS; i++)
		biquad_init(&filters[i]);
}

static void
cleanup_eq(LADSPA_Handle instance) {
	free(instance);
}

static void
connect_port_eq(LADSPA_Handle instance, ulong port, LADSPA_Data *data) {
	eq_t *eq = (eq_t *)instance;
	if (port < BANDS)
		eq->chg[port - BANDS*0] = data;
	else if (port < BANDS*2)
		eq->chf[port - BANDS*1] = data;
	else if (port < BANDS*3)
		eq->chb[port - BANDS*2] = data;
	else if (port == EQ_INPUT)
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

	const LADSPA_Data g = 0;
	const LADSPA_Data f = 440;
	const LADSPA_Data b = 1;
	for (int i = 0; i < BANDS; i++) {
		eq->old_chg[i] = g;
		eq->old_chf[i] = f;
		eq->old_chb[i] = b;
		filters[i] = biquad_gen(1, f, g, b, fs);
	}

	return (LADSPA_Handle) eq;
}

static void
run_eq_for_real(LADSPA_Handle instance, ulong sample_count, int running) {
	eq_t *eq = (eq_t *) instance;

	const LADSPA_Data fs = eq->fs;
	biquad *filters = eq->filters;

	/* TODO: I don't think this should be here at all */
	for (int i = 0; i < BANDS; i++) {
		const LADSPA_Data rg = *(eq->chg[i]);
		const LADSPA_Data rf = *(eq->chf[i]);
		const LADSPA_Data rb = *(eq->chb[i]);
		const LADSPA_Data g = LIMIT(rg, GAIN_MIN, GAIN_MAX);
		const LADSPA_Data f = LIMIT(rf, FREQ_MIN, FREQ_MAX);
		const LADSPA_Data b = LIMIT(rb, BW_MIN, BW_MAX);
		if ((g != eq->old_chg[i])
		 || (f != eq->old_chf[i])
		 || (b != eq->old_chb[i])) {
			eq->old_chg[i] = g;
			eq->old_chf[i] = f;
			eq->old_chb[i] = b;
			filters[i] = biquad_gen(1, f, g, b, fs);
		}
	}

	const LADSPA_Data *input = eq->input;
	LADSPA_Data *output = eq->output;

	for (ulong pos = 0; pos < sample_count; pos++) {
		LADSPA_Data samp = input[pos];
		for (int i = 0; i < BANDS; i++) {
			/* TODO: skip over 0 gains? */
			samp = biquad_run(&filters[i], samp);
		}
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

void
_init() {
	#define INCTRL (LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL)
	#define BOUNDED (LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE)
	for (int i = 0; i < BANDS; i++) {
		const int gi = i;
		const int fi = i + BANDS;
		const int bi = i + BANDS*2;

		p_discs[gi] = INCTRL;
		p_discs[fi] = INCTRL;
		p_discs[bi] = INCTRL;

		/* TODO: generate strings per band */
		p_names[gi] = "Band x Gain [dB]";
		p_names[fi] = "Band x Freq [Hz]";
		p_names[bi] = "Band x Bandwidth [octaves]";

		p_hints[gi].HintDescriptor = BOUNDED
		    | LADSPA_HINT_DEFAULT_0;
		p_hints[fi].HintDescriptor = BOUNDED
		    | LADSPA_HINT_LOGARITHMIC
		    | LADSPA_HINT_DEFAULT_440;
		p_hints[bi].HintDescriptor = BOUNDED
		    | LADSPA_HINT_DEFAULT_1;

		p_hints[gi].LowerBound = GAIN_MIN;
		p_hints[gi].UpperBound = GAIN_MAX;
		p_hints[fi].LowerBound = FREQ_MIN;
		p_hints[fi].UpperBound = FREQ_MAX;
		p_hints[bi].LowerBound = BW_MIN;
		p_hints[bi].UpperBound = BW_MAX;
	}

	p_discs[EQ_INPUT] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
	p_names[EQ_INPUT] = "Input";
	p_hints[EQ_INPUT].HintDescriptor = 0;

	p_discs[EQ_OUTPUT] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	p_names[EQ_OUTPUT] = "Output";
	p_hints[EQ_OUTPUT].HintDescriptor = 0;

	eqDescriptor.activate = activate_eq;
	eqDescriptor.cleanup = cleanup_eq;
	eqDescriptor.connect_port = connect_port_eq;
	eqDescriptor.deactivate = NULL;
	eqDescriptor.instantiate = instantiate_eq;
	eqDescriptor.run = run_eq;
	eqDescriptor.run_adding = run_adding_eq;
	eqDescriptor.set_run_adding_gain = set_run_adding_gain;

	ready = 1;
}

void
_fini() {
}

