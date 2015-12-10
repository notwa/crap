//#INCLUDE
//#REDEFINE

#include <stdio.h>

#include "public.sdk/source/vst2.x/audioeffectx.h"

// VST 2.4 by standard only holds 8 (+ 1 null) length strings,
// but this is never the case in practice. I've seen up to 24.
#define MAX_PARAM_LEN 24

struct plugin : public AudioEffectX
{
	plugin(audioMasterCallback audioMaster);
	virtual ~plugin();

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

	void setParameter(VstInt32, float);
	float getParameter(VstInt32);
	void getParameterName(VstInt32, char *); // eg. Gain
	void getParameterDisplay(VstInt32, char *); // eg. -3.3
	void getParameterLabel(VstInt32, char *); // eg. dB

	char programName[kVstMaxProgNameLen + 1];

	Param *params;

	Crap *crap;
};

AudioEffect *
createEffectInstance(audioMasterCallback audioMaster)
{
	// TODO: return new plugin<CrapPlug>(audioMaster);
	return new plugin(audioMaster);
}

plugin::plugin(audioMasterCallback audioMaster)
: AudioEffectX(audioMaster, 1, CrapPlug::parameters)
{
	crap = new CrapPlug();
	if (CrapPlug::parameters)
		params = new Param[CrapPlug::parameters];
	setNumInputs(2);
	setNumOutputs(2);
	setUniqueID(CrapPlug::id);
	canProcessReplacing();
	canDoubleReplacing();
	vst_strncpy(programName, "Init", kVstMaxProgNameLen);
	CrapPlug::construct_params(params);
}

plugin::~plugin()
{
	delete crap;
	if (CrapPlug::parameters)
		delete[] params;
}

void
plugin::resume()
{
	crap->resume();
	AudioEffectX::resume();
}

void
plugin::suspend()
{
	AudioEffectX::suspend();
	crap->pause();
}

void
plugin::processReplacing(
    float **inputs, float **outputs, VstInt32 count)
{
	crap->process(
	    inputs[0], inputs[1],
	    outputs[0], outputs[1],
	    count);
}

void
plugin::processDoubleReplacing(
    double **inputs, double **outputs, VstInt32 count)
{
	crap->process(
	    inputs[0], inputs[1],
	    outputs[0], outputs[1],
	    count);
}

bool
plugin::setProcessPrecision(VstInt32 precision)
{
	return true;
}

void
plugin::setSampleRate(float fs)
{
	AudioEffectX::setSampleRate(fs);
	crap->adjust(params, (unsigned long) fs);
	#ifdef DELAY
	setInitialDelay(global_delay);
	#endif
}

bool
plugin::getEffectName(char *name)
{
	vst_strncpy(name, CrapPlug::label, kVstMaxEffectNameLen);
	return true;
}

bool
plugin::getProductString(char *text)
{
	vst_strncpy(text, CrapPlug::name, kVstMaxProductStrLen);
	return true;
}

bool
plugin::getVendorString(char *text)
{
	vst_strncpy(text, CrapPlug::author, kVstMaxVendorStrLen);
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

void
plugin::setParameter(VstInt32 index, float value)
{
	if (index >= CrapPlug::parameters) return;
	params[index].set(value);
	crap->adjust_one(params, index);
}

float
plugin::getParameter(VstInt32 index)
{
	if (index >= CrapPlug::parameters) return 0;
	return params[index].get();
}

void
plugin::getParameterName(VstInt32 index, char *text)
{
	if (index >= CrapPlug::parameters) return;
	vst_strncpy(text, params[index].name, MAX_PARAM_LEN);
}

void
plugin::getParameterDisplay(VstInt32 index, char *text)
{
	if (index >= CrapPlug::parameters) return;

	Param *p = &params[index];
	char display[MAX_PARAM_LEN + 1];

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
		sprintf(display, (p->get() < 0.5) ? "off" : "on");
		break;
	default:
		sprintf(display, "error");
	}

	vst_strncpy(text, display, MAX_PARAM_LEN);
}

void
plugin::getParameterLabel(VstInt32 index, char *text)
{
	if (index >= CrapPlug::parameters) return;

	Param *p = &params[index];
	char display[MAX_PARAM_LEN + 1];

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
