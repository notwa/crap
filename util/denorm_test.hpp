#ifndef __arm__
 #ifdef __SSE2__
  #ifndef __SSE2_MATH__
   #warning SSE2 enabled but not forced, beware denormals
  #endif
 #else
  #warning built without SSE2, denormals will be painful
 #endif
#else
 #ifndef __ARM_NEON__
  #warning built without NEON, denormals will be painful
 #else
  #ifndef __FAST_MATH__
   #warning NEON enabled but not forced, beware denormals
  #endif
 #endif
#endif
