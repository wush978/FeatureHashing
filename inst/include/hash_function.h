/*
 * This file is part of FeatureHashing
 * Copyright (C) 2015 Wush Wu
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __HASH_FUNCTION_HPP__
#define __HASH_FUNCTION_HPP__

#include <cstdint>
#include <map>
#include <string>

#ifdef HAVE_VISIBILITY_ATTRIBUTE
# define attribute_hidden __attribute__ ((visibility ("hidden")))
#else
# define attribute_hidden
#endif

extern "C" {

  /* First look for special cases */
#if defined(_MSC_VER)
#define MH_UINT32 unsigned long
#endif
  
/* If the compiler says it's C99 then take its word for it */
#if !defined(MH_UINT32) && ( \
  defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L )
#include <cstdint>
#define MH_UINT32 uint32_t
#endif
    
/* Otherwise try testing against max value macros from limit.h */
#if !defined(MH_UINT32)
#include  <climits>
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
    
MH_UINT32 attribute_hidden PMurHash32(MH_UINT32 seed, const void *key, int len);
    
}

class HashFunction {

public:

  virtual uint32_t operator()(const char* buf, int size, bool is_interaction = false) = 0;

};

#endif
