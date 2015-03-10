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
#include <Rcpp.h>
#include "digestlocal.h"
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

