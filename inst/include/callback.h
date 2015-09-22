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

#ifndef __CALLBACK_H__
#define __CALLBACK_H__

#include <vector>
#include <string>
#include "vector_converter.h"

class CallbackFunctor {

public:

  // TODO: let src private
  Rcpp::CharacterVector src;

  CallbackFunctor(SEXP _src) : src(_src) { }
  virtual ~CallbackFunctor() { }
  
  virtual const std::vector<std::string> operator()(const char* input) const = 0;
  
};

class CallbackConverter : public VectorConverter {
  
  // TODO: refactor this
  Rcpp::CharacterVector src;
  const CallbackFunctor* f;
  SEXP psrc;
  std::vector< std::string > cache;
  
public:
  
  CallbackConverter(const CallbackFunctor* _f, const Param& param)
    : f(_f), src(_f->src), psrc(_f->src), VectorConverter(param) 
    { }
  
  virtual ~CallbackConverter() { }
  
  virtual const std::vector<uint32_t>& get_feature(size_t i) {
    SEXP pstr = STRING_ELT(psrc, i);
    if (pstr == NA_STRING) {
      feature_buffer.clear();
    } else {
      const char* str = CHAR(pstr);
      cache = f->operator()(str);
      feature_buffer.resize(cache.size());
      std::transform(cache.begin(), cache.end(), feature_buffer.begin(), 
                     [this](const std::string& s) {
                       return this->get_hashed_feature(this->h_main, s.c_str());
                     });
      if (is_final) std::transform(feature_buffer.begin(), feature_buffer.end(), 
          feature_buffer.begin(), [this](uint32_t feature) {
            return feature % this->hash_size;
          });
    }
    return feature_buffer;
  }
  
  virtual const std::vector<double>& get_value(size_t i) {
    SEXP pstr = STRING_ELT(psrc, i);
    if (pstr == NA_STRING) {
      value_buffer.clear();
    } else {
      const char* str = CHAR(pstr);
      value_buffer.resize(cache.size());
      std::transform(cache.begin(), cache.end(), value_buffer.begin(), 
                     [this](const std::string& s) {
                       return this->get_hashed_feature(this->h_binary, s.c_str());
                     });
    }
    return value_buffer;
  }
  
};

RCPP_EXPOSED_CLASS(CallbackFunctor)

#endif //__CALLBACK_H__