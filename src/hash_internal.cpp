#include <cstring>
#include <Rcpp.h>
#include "capi.h"
using namespace Rcpp;

//[[Rcpp::export("hash_without_intercept")]]
IntegerVector hash_without_intercept(CharacterVector src) {
  IntegerVector retval(src.size(), 0);
  for(int i = 0;i < src.size();i++) {
    const char* str = CHAR(src[i]);
    if (::strcmp("(Intercept)", str) == 0) continue;
    retval[i] = FeatureHashing_crc32(str, ::strlen(str));
  }
  return retval;
}

//[[Rcpp::export("rehash_inplace")]]
SEXP rehash_inplace(S4 m, IntegerVector Rmapping, int hash_size) {
  uint32_t *pmapping = reinterpret_cast<uint32_t*>(INTEGER(wrap(Rmapping)));
  for(int i = 0;i < Rmapping.size();i++) {
    pmapping[i] = pmapping[i] % hash_size;
  }
  IntegerVector i(m.slot("i")), dimension(m.slot("Dim"));
  dimension[0] = hash_size;
  for(int j = 0;j < i.size();j++) {
    i[j] = pmapping[i[j]];
  }
  return R_NilValue;
}