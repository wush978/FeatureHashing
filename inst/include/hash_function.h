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
#include <Rcpp.h>

class HashFunction {

public:

  virtual uint32_t operator()(const char* buf, int size, bool is_interaction = false) = 0;

};

class NullHashFunction : public HashFunction {
  
  public:
  
  virtual uint32_t operator()(const char* buf, int size, bool is_interaction = false);

};

class MurmurHash3HashFunction : public HashFunction {
  
  uint32_t seed;
  
public :

  MurmurHash3HashFunction(uint32_t _seed) : seed(_seed) { }

  virtual uint32_t operator()(const char* buf, int size, bool is_interaction = false);

};

class MurmurHash3LogHashFunction : public HashFunction {
  
  uint32_t seed;
  Rcpp::Environment e;
  std::map<uint32_t, std::string> inverse_mapping;
  
public:

  MurmurHash3LogHashFunction(SEXP _e, uint32_t _seed) 
  : HashFunction(), seed(_seed), e(_e)
  { }
  
  virtual uint32_t operator()(const char* buf, int size, bool is_interaction = false);
  
};

#endif