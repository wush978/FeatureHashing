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

#ifndef __VECTOR_CONVERTER_HPP__
#define __VECTOR_CONVERTER_HPP__

#include "callback.h"
#include "hash_function.h"
#include <RcppCommon.h>

struct VectorConverterParam;
class VectorConverter;

typedef VectorConverterParam Param;

/**
 * Paramter of initializing VectorConverter
 */
struct VectorConverterParam {
  
  std::string name;
  HashFunction* h_main;
  HashFunction* h_binary;
  size_t hash_size;

  VectorConverterParam(const std::string& _name, HashFunction* _h_main, HashFunction* _h_binary, size_t _hash_size)
  : name(_name), h_main(_h_main), h_binary(_h_binary), hash_size(_hash_size) 
  { }
  
};

class VectorConverter {

protected:
  std::vector<uint32_t> feature_buffer;
  std::vector<double> value_buffer;
  std::string name;
  size_t name_len;
  HashFunction *h_main, *h_binary;
  size_t hash_size;

public:

  bool is_final;
  
  explicit VectorConverter(const Param& param) 
  : name(param.name), name_len(param.name.size()), h_main(param.h_main), 
  h_binary(param.h_binary), hash_size(param.hash_size), is_final(true)
  { }
  
  virtual ~VectorConverter() { }
  
  /**
   * Evaluate the hashed feature from the raw data.
   * The \code{get_feature} should be called before \code{get_value}
   */
  virtual const std::vector<uint32_t>& get_feature(size_t i) = 0;
  
  /**
   * Evaluate the value from the raw data
   * The \code{get_feature} should be called before \code{get_value}
   */ 
  virtual const std::vector<double>& get_value(size_t i) = 0;
  
  const std::string& get_name() const {
    return name;
  }
  
protected:

  /**
   * @return -1 if (int) v < 0, or +1 otherwise
   */ 
  static inline int get_sign(uint32_t v) {
    if ((int) v < 0) return -1;
    else return 1;
  }

  uint32_t get_hashed_feature(HashFunction* h, const char* str) {
    name.append(str);
    #ifdef NOISY_DEBUG
    Rprintf("hashing %s ... ", name.c_str());
    #endif
    uint32_t retval = (*h)(name.c_str(), name.size());
    #ifdef NOISY_DEBUG
    Rprintf(" got %zu \n", retval);
    #endif
    name.resize(name_len);
    return retval;
  }

};

  
#endif // __VECTOR_CONVERTER_HPP__