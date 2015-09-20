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

#ifndef __SPLIT_H__
#define __SPLIT_H__

#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include "callback.h"

std::vector<std::string> split(const std::string& src, const std::string& delim);

enum class SplitType {
  Count,
  Existence
};

class SplitCallbackFunctor : public CallbackFunctor {
  
  SplitType type;
  const std::string delim;

  const std::vector<std::string> split_count(const char* input) const {
    std::vector<std::string> temp(split(input, delim));
    temp.erase(std::remove(temp.begin(), temp.end(), ""), temp.end());
    return temp;
  }
  
  const std::vector<std::string> split_existence(const char* input) const {
    std::vector<std::string> temp(split(input, delim));
    std::set<std::string> temp2(temp.begin(), temp.end());
    temp2.erase("");
    temp.assign(temp2.begin(), temp2.end());
    return temp;
  }
  
public:
  
  explicit SplitCallbackFunctor(const std::string& _delim, SplitType _type)
    : type(_type), delim(_delim), CallbackFunctor()
    { }
  
  virtual ~SplitCallbackFunctor() { }
  
  virtual const std::vector<std::string> operator()(const char* input) const {
    switch (type) {
    case SplitType::Count : 
      return split_count(input);
    case SplitType::Existence :
      return split_existence(input);
    }
  }
  
};

#endif //__SPLIT_H__
