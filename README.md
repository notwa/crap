# crap

connor's rancid audio plugins. LADSPA and VST.

this is a set of hacked-together hacks to be hacked on
in the personal interest of learning and experimenting.

MIT-licensed; go wild.

[eigen]: http://eigen.tuxfamily.org/
[moog]: https://aaltodoc.aalto.fi/bitstream/handle/123456789/14420/article6.pdf
[mxe]: //mxe.cc
[msys2]: //msys2.github.io

## plugs

Name | Label | ID | Description
--- | --- | --- | ---
crap Parametric Equalizer  | crap\_eq | `0x000CAFED` | multiband parametric EQ. try redefining BANDS. #define BANDS to desired number of equalizer bands; default 4.
crap sample delay test | crap\_delay\_test | `0xDEDEDEDE` | why did i commit this?
crap Constant Equalizer | crap\_eq\_const | `0x0DEFACED` | simpler code than crap\_eq for a static response. edit code as needed.
crap T420 Speaker Compensation | crap\_eq\_const\_T420 | `0x0DEFAE91` | lenovo tests
crap T420 Speaker Compensation (SVF) | crap\_eq\_const\_T420\_svf | `0x0DEFB3CA` | trying out State Variable Filters (SVFs)
crap mugi4 (moog-like) | crap\_mugi4 | `0xD8D0D8D0` | nonlinear moog filter implementation: [see reference][moog]
crap Tube Distortion | crap\_tube | `0x50F7BA11` | static waveshaper with 6x oversampling and parameter smoothing. doesn't actually emulate a tube. not actually for crapping in.
crap Leveller | crap\_level | `0xAAAAAAAA` | an early prototype of a heavy leveller.

## build notes

### dependencies

crap is now dependent on [the Eigen header library.][eigen]
for your convenience, a `get_eigen` bash script is provided.
this script simply downloads and extracts the Eigen header files
required to build crap.

you may need to add `-fabi-version=6` to CXXFLAGS if you use g++.

### general building

`make linux` or `make windows`
(works well with [mxe][mxe] and [msys2 mingw64][msys2])

`make linux-arm` requires a CPU with a NEON unit

**remember to export** `VST_SDK_DIR` **to the path of your** `vstsdk2.4/`

other targets:
* all: no hand-holding.
* design: simple CLI to the biquad coefficient calculator.
* benchmark: does what it says on the tin. no Windows support.

## TODO

* LV2 support
* automatically generate markdown table based on crap contents
* rename plugins (fix capitalization consistency and such)
* fix code style; particularily braces in method definitions and spaces in casts
* scrap overly-complex makefile for a shell script
* don't mix CamelCase with underscores (only done for legacy reasons)
