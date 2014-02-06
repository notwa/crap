# crap

connor's rancid audio plugins. LADSPA and VST.

alternatively, configuration realizes any personality.

## plugs

### crap Parametric Equalizer

__crap\_eq (0x000CAFED)__

multiband parametric EQ. try redefining BANDS.

### crap const Equalizer

__crap\_eq\_const (0x0DEFACED)__

simpler code with a static response.
edit code as needed.

### crap noise generator

__crap\_noise (0xEC57A71C)__

white noise generator. loud, full-range, 0dBFS. don't say i didn't warn you.

### crap Tube Distortion

__crap\_tube (0x50F7BA11)__

static waveshaper with 4x oversampling, sounds kinda like a tube I guess?
be aware that the oversampling is a bit naive and attenuates the signal
past 17kHz, assuming a 44.1kHz sample rate.

### crap delay test

__crap\_delay\_test (0xDEDEDEDE)__

experimentation with delay compensation and EQ oversampling, not for use.

## build notes

`make` it.

a `benchmark` target is included, however it doesn't build on Windows.

for speed, try `CFLAGS="-O3 -ffast-math -march=core2 -mfpmath=sse"`
and the same for CXXFLAGS.

on Linux, you'll need `CFLAGS+=" -fpic" CXXFLAGS+=" -fpic -D__cdecl="`

remember to export `VST_SDK_DIR` to the path of your `vstsdk2.4/`

## TODO

* rename plugins (fix capitalization consistency and such)
* reduce input/output buffers on biquads (shared)
* ease up on the preprocessor ifs
* polish parameter support
* make code style consistent
