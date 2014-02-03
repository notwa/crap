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

### crap delay test

__crap\_delay\_test (0xDEDEDEDE)__

experimentation with delay compensation and EQ oversampling, not for use.

## build notes

`make` it.

a `benchmark` target is included, however it doesn't build on Windows.

try `CFLAGS="-O3 -ffast-math -march=core2"`

on Linux, you'll need `CFLAGS+=" -fpic" CXXFLAGS+=" -fpic -D__cdecl="`

remember to export `VST_SDK_DIR` to the path of your `vstsdk2.4/`

## TODO

* convert crap\_noise to the template format
* rename plugins (fix capitalization consistency and such)
* make style consistent
* remove crap\_ prefixes?
* move to subdirs?
* make crap faster (hand-written SSE2? compiler directives?)
* reduce input/output buffers on biquads (shared)
* ease up on the preprocessor ifs
* polish parameter support

