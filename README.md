# crap

connor's rancid audio plugins. LADSPA and VST.

alternatively, configuration realizes any personality.

this is a set of hacked-together hacks to be hacked on
in the personal interest of learning and experimenting.
you should look elsewhere if you need an *enterprize-quality*
cross-platform library to build *lean & agile socially-networked* audio apps.

MIT-licensed; go wild.

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

static waveshaper with 6x oversampling, sounds kinda like a tube i guess?

### crap delay test

__crap\_delay\_test (0xDEDEDEDE)__

experimentation with delay compensation and EQ oversampling, not for use.

## build notes

`make linux` or `make windows` (works well with [mxe])
[mxe]: //mxe.cc

remember to export `VST_SDK_DIR` to the path of your `vstsdk2.4/`

other targets:
* all: no hand-holding.
* design: simple CLI to the biquad coefficient calculator.
* benchmark: does what it says on the tin. no Windows support.

## TODO

* ARM support
* spaces-in-paths support in Makefile
* rename plugins (fix capitalization consistency and such)
* reduce input/output buffers on biquads (shared)
* ease up on the preprocessor ifs
* polish parameter support
* make code style consistent
* perhaps LV2 or AU support
