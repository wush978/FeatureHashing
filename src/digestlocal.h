/**
 * 
 * MurmurHash3 was written by Austin Appleby, and is placed in the public.
 * 
 * This header links the implementation of murmurhash3 in digest3 to FeatureHashing.
 * This was writting by Wush Wu, and also public domain.
 * 
 */
 
#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>
#include <Rconfig.h>
#include <R_ext/Rdynload.h>

#ifdef HAVE_VISIBILITY_ATTRIBUTE
    # define attribute_hidden __attribute__ ((visibility ("hidden")))
#else
    # define attribute_hidden
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* First look for special cases */
#if defined(_MSC_VER)
  #define MH_UINT32 unsigned long
#endif

/* If the compiler says it's C99 then take its word for it */
#if !defined(MH_UINT32) && ( \
     defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L )
  #include <stdint.h>
  #define MH_UINT32 uint32_t
#endif

/* Otherwise try testing against max value macros from limit.h */
#if !defined(MH_UINT32)
  #include  <limits.h>
  #if   (USHRT_MAX == 0xffffffffUL)
    #define MH_UINT32 unsigned short
  #elif (UINT_MAX == 0xffffffffUL)
    #define MH_UINT32 unsigned int
  #elif (ULONG_MAX == 0xffffffffUL)
    #define MH_UINT32 unsigned long
  #endif
#endif

#if !defined(MH_UINT32)
  #error Unable to determine type name for unsigned 32-bit int
#endif

/* I'm yet to work on a platform where 'unsigned char' is not 8 bits */
#define MH_UINT8  unsigned char

MH_UINT32 PMurHash32(MH_UINT32, const void*, int);

extern const MH_UINT32 MURMURHASH3_H_SEED, MURMURHASH3_XI_SEED;

#ifdef __cplusplus
}
#endif
