/*
 * This file is part of FeatureHashing
 * Copyright (C) 2014-2015 Wush Wu
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

#include <cstring>
#include <deque>
#include <boost/detail/endian.hpp>
#include <boost/algorithm/string.hpp>
#include <Rcpp.h>
#include "hash_function.h"
using namespace Rcpp;

//'@export hash.sign
//[[Rcpp::export("hash.sign")]]
IntegerVector xi(CharacterVector src) {
  IntegerVector retval(src.size(), 0);
  for(int i = 0;i < src.size();i++) {
    const char* str = CHAR(src[i]);
    if (::strcmp("(Intercept)", str) == 0) continue;
    retval[i] = (int) PMurHash32(MURMURHASH3_XI_SEED, str, ::strlen(str));
    if (retval[i] < 0) retval[i] = -1;
    else retval[i] = 1;
  }
  return retval;
}

//'@export hashed.value
//[[Rcpp::export("hashed.value")]]
IntegerVector h(CharacterVector src) {
  IntegerVector retval(src.size(), 0);
  for(int i = 0;i < src.size();i++) {
    const char* str = CHAR(src[i]);
    if (::strcmp("(Intercept)", str) == 0) continue;
    retval[i] = PMurHash32(MURMURHASH3_H_SEED, str, ::strlen(str));
  }
  return retval;
}

MH_UINT32 interaction(MH_UINT32 a, MH_UINT32 b) {
  MH_UINT32 buf[2];
#ifdef BOOST_BIG_ENDIAN
  buf[0] = bswap_32(a);
  buf[1] = bswap_32(b);
#else
  buf[0] = a;
  buf[1] = b;
#endif
  return PMurHash32(MURMURHASH3_H_SEED, reinterpret_cast<char*>(buf), sizeof(MH_UINT32) * 2);
}

//'@export hashed.interaction.value
//[[Rcpp::export("hashed.interaction.value")]]
IntegerVector h2(CharacterVector src) {
  IntegerVector retval(src.size(), 0);
  std::vector<std::string> tokens;
  std::deque<MH_UINT32> values;
  for(int i = 0;i < src.size();i++) {
    const char* str = CHAR(src[i]);
    boost::split(tokens, str, boost::is_any_of(":"));
    values.resize(tokens.size());
    for(int j = 0;j < tokens.size();j++) {
      values[j] = PMurHash32(MURMURHASH3_H_SEED, tokens[j].c_str(), tokens[j].size());
    }
    MH_UINT32 value = values[0];
    values.pop_front();
    while(values.size() > 0) {
      value = interaction(value, values[0]);
      values.pop_front();
    }
    retval[i] = value;
  }
  return retval;
}

