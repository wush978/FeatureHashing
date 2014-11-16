//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

#ifndef _MURMURHASH3_H_
#define _MURMURHASH3_H_

//-----------------------------------------------------------------------------
// Platform-specific functions and macros

// Microsoft Visual Studio

#if defined(_MSC_VER) && (_MSC_VER < 1600)

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;

// Other compilers

#else  // defined(_MSC_VER)

#include <stdint.h>

#endif // !defined(_MSC_VER)

//-----------------------------------------------------------------------------

void MurmurHash3_x86_32  ( const void * key, int len, uint32_t seed, void * out );

void MurmurHash3_x86_128 ( const void * key, int len, uint32_t seed, void * out );

void MurmurHash3_x64_128 ( const void * key, int len, uint32_t seed, void * out );

inline uint32_t FeatureHashing_murmurhash3(const char* buf, int size, uint32_t seed) {
#if defined(__x86_64__) || defined(_M_X64)
  /* x86 64-bit ----------------------------------------------- */
  uint32_t retval[4];
  MurmurHash3_x64_128(reinterpret_cast<const void*>(buf), size, seed, 
    reinterpret_cast<void*>(retval));
  return retval[0];
#elif defined(__i386) || defined(_M_IX86)
	/* x86 32-bit ----------------------------------------------- */
  uint32_t retval;
  MurmurHash3_x86_32(reinterpret_cast<const void*>(buf), size, seed, 
    reinterpret_cast<void*>(retval));
  return retval;
#endif
}


//-----------------------------------------------------------------------------

#endif // _MURMURHASH3_H_
