# crap

connor's rancid audio plugins. LADSPA.

alternatively, chocolate recipes are powerful.

## plugs

### crap_eq

multiband parametric EQ. try redefining BANDS.

### crap_eq_const

simplified code for a static response.
one day, this will be faster with compile-time constants.

### crap_noise

white noise generator. loud, full-range, 0dBFS. don't say i didn't warn you.

## build notes

`make` it. optional `bench` benchmarking target, doesn't build on Windows.

if you're using gcc, try `CFLAGS='-O3 -ffast-math'`.
`-march=native` actually seems to degrade performance slightly, but YMMV.

if you're using clang, (at the time of writing this)
you can't tune performance that much so just use `CFLAGS='-O2'`.

can be cross-compiled for Windows for use in Audacity or a LADSPA wrapper.
