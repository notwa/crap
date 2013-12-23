#include "public.sdk/source/vst2.x/audioeffectx.h"
//#include "vstparam.h"

//#INCLUDE

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

	/*
	void setParameter(VstInt32, float);
	float getParameter(VstInt32);
	void getParameterLabel(VstInt32, char *);
	void getParameterDisplay(VstInt32, char *);
	void getParameterName(VstInt32, char *);
	*/

	void setSampleRate(float);
	bool getEffectName(char *);
	bool getVendorString(char *);
	bool getProductString(char *);
	//VstInt32 getVendorVersion();

private:
	#if (PARAMETERS > 0)
	VstParam *m_params[PARAMETERS];
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
//#VST_PARAMS
	//m_params[n] = new VstParam("Input", "dB", -12.0, 12.0, 0.0, NULL, NULL, 1, NULL);
	setNumInputs(2);
	setNumOutputs(2);
	setUniqueID(ID);
	canProcessReplacing();
	::construct(&data);
}

plugin::~plugin()
{
	::destruct(&data);
	//for (int i = 0; i < PARAMETERS; i++)
	//    delete m_params[i];
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

/*
parameter funcs go here
    if (index >= PARAMETERS) return;
*/

void
plugin::setSampleRate(float fs) {
	AudioEffectX::setSampleRate(fs);
	::adjust(&data, (unsigned long) fs);
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
