# crap

connor's rancid audio plugins. LADSPA and VST.

alternatively, configuration realizes any personality

## plugs

### crap Parametric Equalizer

__crap_eq (0x000CAFED)__

multiband parametric EQ. try redefining BANDS.

### crap const Equalizer

__crap_eq_const (0x0DEFACED)__

simpler code with a static response.
edit code as needed.

### crap noises generator

__crap_noise (0xEC57A71C)__

white noise generator. loud, full-range, 0dBFS. don't say i didn't warn you.

## build notes

`make` it. optional `benchmark` target which doesn't build on Windows.

with gcc, try `CFLAGS='-O3 -ffast-math'`.
`-march=native` seems to degrade performance slightly, but YMMV.

with clang, (at the time of writing this)
optimizations are not as tuneable so try `CFLAGS='-O2'`.

can be cross-compiled for Windows for use in Audacity or a LADSPA wrapper.
