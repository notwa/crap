#ifdef __SSE2__
#ifndef __SSE2_MATH__
#warning SSE2 enabled but not forced, beware denormals
#endif
#else
#warning built without SSE2, denormals will be painful
#endif
