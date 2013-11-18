#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

void __attribute__ ((constructor)) eq_init();

static const char *gain_desc = "Band %i Gain [dB]";
static const char *freq_desc = "Band %i Freq [Hz]";
static const char *band_desc = "Band %i Bandwidth [octaves]";

#define NAME_SIZE 32
LADSPA_PortDescriptor p_discs[PCOUNT];
LADSPA_PortRangeHint p_hints[PCOUNT];
char name_buffer[PCOUNT*NAME_SIZE];
char *p_names[PCOUNT];

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
} eq_t;

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
watch_parameters(eq_t *eq) {
	biquad *filters = eq->filters;
	const LADSPA_Data fs = eq->fs;
	for (int i = 0; i < BANDS; i++) {
		const LADSPA_Data rg = *(eq->chg[i]);
		const LADSPA_Data rf = *(eq->chf[i]);
		const LADSPA_Data rb = *(eq->chb[i]);
		if ((rg != eq->old_chg[i])
		 || (rf != eq->old_chf[i])
		 || (rb != eq->old_chb[i])) {
			const LADSPA_Data g = LIMIT(rg, GAIN_MIN, GAIN_MAX);
			const LADSPA_Data f = LIMIT(rf, FREQ_MIN, FREQ_MAX);
			const LADSPA_Data b = LIMIT(rb, BW_MIN, BW_MAX);
			eq->old_chg[i] = g;
			eq->old_chf[i] = f;
			eq->old_chb[i] = b;
			filters[i] = biquad_gen(1, f, g, b, fs);
		}
	}
}

static void
run_eq(LADSPA_Handle instance, ulong sample_count) {
	eq_t *eq = (eq_t *) instance;
	biquad *filters = eq->filters;

	watch_parameters(eq);

	const LADSPA_Data *input = eq->input;
	LADSPA_Data *output = eq->output;

	for (ulong pos = 0; pos < sample_count; pos++) {
		LADSPA_Data samp = input[pos];
		for (int i = 0; i < BANDS; i++)
			samp = biquad_run(&filters[i], samp);
		output[pos] = samp;
	}
}

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
	.PortNames = p_names,

	.activate = activate_eq,
	.cleanup = cleanup_eq,
	.connect_port = connect_port_eq,
	.deactivate = NULL,
	.instantiate = instantiate_eq,
	.run = run_eq,
	.run_adding = NULL,
	.set_run_adding_gain = NULL
};

const LADSPA_Descriptor *
ladspa_descriptor(ulong index) {
	if (index != 0)
		return NULL;
	return &eqDescriptor;
}

void
eq_init() {
	#define INCTRL (LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL)
	#define BOUNDED (LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE)
	for (int i = 0; i < PCOUNT; i++)
		p_names[i] = &name_buffer[i*NAME_SIZE];
	for (int i = 0; i < BANDS; i++) {
		const int gi = i;
		const int fi = i + BANDS;
		const int bi = i + BANDS*2;

		p_discs[gi] = INCTRL;
		p_discs[fi] = INCTRL;
		p_discs[bi] = INCTRL;

		sprintf(p_names[gi], gain_desc, i);
		sprintf(p_names[fi], freq_desc, i);
		sprintf(p_names[bi], band_desc, i);

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
	strcpy(p_names[EQ_INPUT], "Input");
	p_hints[EQ_INPUT].HintDescriptor = 0;

	p_discs[EQ_OUTPUT] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
	strcpy(p_names[EQ_OUTPUT], "Output");
	p_hints[EQ_OUTPUT].HintDescriptor = 0;
}
