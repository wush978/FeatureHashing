#include <cstring>
#include <Rcpp.h>
#include "digestlocal.h"
using namespace Rcpp;

//'@export hash_xi
//[[Rcpp::export("hash_xi")]]
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

//'@export hash_h
//[[Rcpp::export("hash_h")]]
IntegerVector h(CharacterVector src) {
  IntegerVector retval(src.size(), 0);
  for(int i = 0;i < src.size();i++) {
    const char* str = CHAR(src[i]);
    if (::strcmp("(Intercept)", str) == 0) continue;
    retval[i] = PMurHash32(MURMURHASH3_H_SEED, str, ::strlen(str));
  }
  return retval;
}

