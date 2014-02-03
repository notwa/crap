#include <stdio.h>
#include "public.sdk/source/vst2.x/audioeffectx.h"

//#INCLUDE

// VST 2.4 by standard only holds 8 (+ 1 null) length strings,
// but this is never the case in practice. I've seen up to 24.
#define MAX_PARAM_LEN 24

class plugin : public AudioEffectX
{
public:
	plugin(audioMasterCallback audioMaster);
	~plugin();

	void resume();
	void suspend();
	void processReplacing(float **, float **, VstInt32);
	void processDoubleReplacing(double **, double **, VstInt32);

	bool setProcessPrecision(VstInt32 precision);

	void setSampleRate(float);
	void setProgramName(char *);
	void getProgramName(char *);
	bool getProgramNameIndexed(VstInt32, VstInt32, char *);
	bool getEffectName(char *);
	bool getVendorString(char *);
	bool getProductString(char *);
	//VstInt32 getVendorVersion();

	#if (PARAMETERS > 0)
	void setParameter(VstInt32, float);
	float getParameter(VstInt32);
	void getParameterName(VstInt32, char *); // eg. Gain
	void getParameterDisplay(VstInt32, char *); // eg. -3.3
	void getParameterLabel(VstInt32, char *); // eg. dB
	#endif

private:
	char programName[kVstMaxProgNameLen];

	#if (PARAMETERS > 0)
	param params[PARAMETERS];
	#endif

	personal data;
};

AudioEffect *
createEffectInstance(audioMasterCallback audioMaster) {
	return new plugin(audioMaster);
}

plugin::plugin(audioMasterCallback audioMaster)
: AudioEffectX(audioMaster, 1, PARAMETERS)
{
	setNumInputs(2);
	setNumOutputs(2);
	setUniqueID(ID);
	canProcessReplacing();
	canDoubleReplacing();
	vst_strncpy(programName, "Init", kVstMaxProgNameLen);
	#if (PARAMETERS > 0)
	::construct_params(params);
	#endif
	::construct(&data);
}

plugin::~plugin()
{
	::destruct(&data);
}

void
plugin::resume() {
	::resume(&data);
	AudioEffectX::resume();
}

void
plugin::suspend() {
	AudioEffectX::suspend();
	::pause(&data);
}

void
plugin::processReplacing(float **inputs, float **outputs, VstInt32 count) {
	::process(&data,
	    inputs[0], inputs[1],
	    outputs[0], outputs[1],
	    count);
}

void
plugin::processDoubleReplacing(double **inputs, double **outputs, VstInt32 count) {
	::process_double(&data,
	    inputs[0], inputs[1],
	    outputs[0], outputs[1],
	    count);
}

bool
plugin::setProcessPrecision(VstInt32 precision) {
	return true;
}

void
plugin::setSampleRate(float fs) {
	AudioEffectX::setSampleRate(fs);
	#if (PARAMETERS > 0)
	::adjust(&data, params, (unsigned long) fs);
	#else
	::adjust(&data, (unsigned long) fs);
	#endif
	#ifdef DELAY
	setInitialDelay(global_delay);
	#endif
}

bool
plugin::getEffectName(char *name) {
	vst_strncpy(name, LABEL, kVstMaxEffectNameLen);
	return true;
}

bool
plugin::getProductString(char *text)
{
	vst_strncpy(text, NAME, kVstMaxProductStrLen);
	return true;
}

bool
plugin::getVendorString(char *text)
{
	vst_strncpy(text, AUTHOR, kVstMaxVendorStrLen);
	return true;
}

void
plugin::setProgramName (char *text)
{
	vst_strncpy(programName, text, kVstMaxProgNameLen);
}

void
plugin::getProgramName (char *text)
{
	vst_strncpy(text, programName, kVstMaxProgNameLen);
}

bool
plugin::getProgramNameIndexed(VstInt32 category, VstInt32 index, char *text)
{
	if (index != 0) return false;
	vst_strncpy(text, programName, kVstMaxProgNameLen);
	return true;
}

#if (PARAMETERS > 0)
void
plugin::setParameter(VstInt32 index, float value)
{
	if (index >= PARAMETERS) return;
	param_set(&params[index], value);
	::adjust_one(&data, params, index);
}

float
plugin::getParameter(VstInt32 index)
{
	if (index >= PARAMETERS) return 0;
	return param_get(&params[index]);
}

void
plugin::getParameterName(VstInt32 index, char *text)
{
	if (index >= PARAMETERS) return;
	vst_strncpy(text, params[index].name, MAX_PARAM_LEN);
}

void
plugin::getParameterDisplay(VstInt32 index, char *text)
{
	if (index >= PARAMETERS) return;

	param *p = &params[index];
	char display[MAX_PARAM_LEN];

	switch (p->scale) {
	case SCALE_FLOAT:
	case SCALE_LOG:
	case SCALE_HZ:
	case SCALE_DB:
		sprintf(display, "%0.2f", p->value);
		break;
	case SCALE_INT:
		sprintf(display, "%i", (int) p->value);
		break;
	case SCALE_TOGGLE:
		sprintf(display, (param_get(p) < 0.5) ? "off" : "on");
		break;
	default:
		sprintf(display, "error");
	}

	vst_strncpy(text, display, MAX_PARAM_LEN);
}

void
plugin::getParameterLabel(VstInt32 index, char *text)
{
	if (index >= PARAMETERS) return;

	param *p = &params[index];
	char display[MAX_PARAM_LEN];

	switch (p->scale) {
	case SCALE_HZ:
		sprintf(display, "Hz");
		break;
	case SCALE_DB:
		sprintf(display, "dB");
		break;
	case SCALE_FLOAT:
	case SCALE_INT:
	case SCALE_TOGGLE:
	case SCALE_LOG:
		display[0] = 0;
	}

	vst_strncpy(text, display, MAX_PARAM_LEN);
}
#endif
