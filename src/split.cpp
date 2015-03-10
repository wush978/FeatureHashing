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
#include "split.h"
using namespace Rcpp;

std::vector<std::string> split(const std::string& src, const std::string& delim) {
  const char* start = src.c_str();
  const char* end = std::strstr(start, delim.c_str());
  std::vector<std::string> retval;
  while(end != NULL) {
    retval.push_back(std::string(start, end));
    start = end + delim.size();
    end = std::strstr(start, delim.c_str());
  }
  retval.push_back(std::string(start));
  return retval;
}

//[[Rcpp::export]]
SEXP split_existence(CharacterVector src, const std::string& delim) {
  std::map<std::string, LogicalVector> retval_buffer;
  for(auto i = 0;i < src.size();i++) {
    std::vector<std::string> tokens(split(CHAR(STRING_ELT(src, i)), delim));
    for(auto j = tokens.begin();j != tokens.end();j++) {
      if (j->size() == 0) continue;
      auto k = retval_buffer.find(*j);
      if (k == retval_buffer.end()) {
        auto tmp = retval_buffer.insert(std::pair<std::string, LogicalVector>(*j, LogicalVector(src.size(), 0)));
        k = tmp.first;
      }
      k->second[i] = 1;
    }
  }
  List retval(retval_buffer.size());
  CharacterVector retval_name(retval_buffer.size());
  size_t j = 0;
  for(auto i = retval_buffer.begin();i != retval_buffer.end();i++) {
    retval[j] = i->second;
    retval_name[j++] = i->first;
  }
  retval.attr("names") = retval_name;
  return retval;
}

//[[Rcpp::export]]
SEXP split_count(CharacterVector src, const std::string& delim) {
  std::map<std::string, IntegerVector> retval_buffer;
  for(auto i = 0;i < src.size();i++) {
    std::vector<std::string> tokens(split(CHAR(STRING_ELT(src, i)), delim));
    for(auto j = tokens.begin();j != tokens.end();j++) {
      if (j->size() == 0) continue;
      auto k = retval_buffer.find(*j);
      if (k == retval_buffer.end()) {
        auto tmp = retval_buffer.insert(std::pair<std::string, IntegerVector>(*j, IntegerVector(src.size(), 0)));
        k = tmp.first;
      }
      k->second[i] += 1;
    }
  }
  List retval(retval_buffer.size());
  CharacterVector retval_name(retval_buffer.size());
  size_t j = 0;
  for(auto i = retval_buffer.begin();i != retval_buffer.end();i++) {
    retval[j] = i->second;
    retval_name[j++] = i->first;
  }
  retval.attr("names") = retval_name;
  return retval;  
}
