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
#include <Rcpp.h>

class CallbackFunctor {

public:

  // TODO: let src private
  Rcpp::CharacterVector src;
  bool decollision;

  CallbackFunctor(SEXP _src) : src(_src), decollision(false) { }
  virtual ~CallbackFunctor() { }
  
  virtual const std::vector<std::string> operator()(const char* input) const = 0;
  
};

#endif //__CALLBACK_H__