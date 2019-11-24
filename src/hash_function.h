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

#include <boost/predef/other/endian.h>
#include <boost/endian/conversion.hpp>
#include "digestlocal.h"

class HashFunction {

public:
  
  virtual ~HashFunction() { }

  virtual uint32_t operator()(const char* buf, int size, bool is_interaction = false) = 0;

};

class NullHashFunction : public HashFunction {
  
  public:
    
  virtual ~NullHashFunction() { }
  
  virtual uint32_t operator()(const char* buf, int size, bool is_interaction = false) {
    return 1;
  }
  
};

class MurmurHash3HashFunction : public HashFunction {
  
  uint32_t seed;
  
public :

  MurmurHash3HashFunction(uint32_t _seed) : seed(_seed) { }
  
  virtual ~MurmurHash3HashFunction() { }

  virtual uint32_t operator()(const char* buf, int size, bool is_interaction = false) {
    return ::PMurHash32(seed, buf, size);
  }
};

class MurmurHash3LogHashFunction : public HashFunction {
  
  uint32_t seed;
  Rcpp::Environment e;
  std::map<uint32_t, std::string> inverse_mapping;
  
public:

  MurmurHash3LogHashFunction(SEXP _e, uint32_t _seed) 
  : HashFunction(), seed(_seed), e(_e)
  { }
  
  virtual ~MurmurHash3LogHashFunction() { }
  
  virtual uint32_t operator()(const char* buf, int size, bool is_interaction = false) {
    uint32_t retval = PMurHash32(seed, buf, size);
    if (is_interaction) {
      const uint32_t* src = reinterpret_cast<const uint32_t*>(buf);
      #if BOOST_ENDIAN_BIG_BYTE && !BOOST_ENDIAN_LITTLE_BYTE
      if (inverse_mapping.find(boost::endian::endian_reverse(src[0])) == inverse_mapping.end()) throw std::logic_error("interaction is hashed before main effect!");
      if (inverse_mapping.find(boost::endian::endian_reverse(src[1])) == inverse_mapping.end()) throw std::logic_error("interaction is hashed before main effect!");
      std::string key(inverse_mapping[boost::endian::endian_reverse(src[0])]);
      key.append(":");
      key.append(inverse_mapping[boost::endian::endian_reverse(src[1])]);
      #elif !BOOST_ENDIAN_BIG_BYTE && BOOST_ENDIAN_LITTLE_BYTE
      if (inverse_mapping.find(src[0]) == inverse_mapping.end()) throw std::logic_error("interaction is hashed before main effect!");
      if (inverse_mapping.find(src[1]) == inverse_mapping.end()) throw std::logic_error("interaction is hashed before main effect!");
      std::string key(inverse_mapping[src[0]]);
      key.append(":");
      key.append(inverse_mapping[src[1]]);
      #else
      #error Unknown endianness
      #endif
      e[key.c_str()] = Rcpp::wrap((int) retval);
      inverse_mapping[retval] = key;
    } 
    else {
      e[buf] = Rcpp::wrap((int) retval);
      inverse_mapping[retval] = buf;
    }
    return retval;
  }
  
};

#endif