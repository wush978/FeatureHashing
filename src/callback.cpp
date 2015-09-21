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

#include "callback.h"
#include "split.h"
#include <Rcpp.h>

using namespace Rcpp;

//'@title Test the callback function.
//'@param Rcallback external pointer. The pointer of the callback function.
//'@param input string. The input.
//'@details The Rcallback is an external pointer which points to a functional pointer..
//'The signature of the functional pointer should be:
//'\code{std::vector<std::string> (*f)(const char* str)}
//'@return character
//'@export
//[[Rcpp::export]]
SEXP test_callback(SEXP Rcallback, const std::string& input) {
  CallbackFunctor* callback(as<CallbackFunctor*>(Rcallback));
  return wrap((*callback)(input.c_str()));
  // return R_NilValue;
}

RCPP_MODULE(callback) {
  
  class_<CallbackFunctor>("callback")
  ;

}
