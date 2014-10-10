#include <cstring>
#include <Rcpp.h>
#include "capi.h"
using namespace Rcpp;

//[[Rcpp::export("hash_without_intercept_single")]]
IntegerVector hash_without_intercept_single(CharacterVector src) {
  IntegerVector retval(src.size(), 0);
  for(int i = 0;i < src.size();i++) {
    const char* str = CHAR(src[i]);
    if (::strcmp("(Intercept)", str) == 0) continue;
    retval[i] = FeatureHashing_crc32(str, ::strlen(str));
  }
  return retval;
}

//[[Rcpp::export("hash_without_intercept")]]
IntegerVector hash_without_intercept(CharacterVector src) {
  IntegerVector retval(src.size(), 0);
  #pragma omp parallel for
  for(int i = 0;i < src.size();i++) {
    const char* str = CHAR(src[i]);
    if (::strcmp("(Intercept)", str) == 0) continue;
    retval[i] = FeatureHashing_crc32(str, ::strlen(str));
  }
  return retval;
}

//[[Rcpp::export("rehash_inplace")]]
void rehash_inplace(S4 m, IntegerVector Rmapping, int hash_size) {
  uint32_t *pmapping = reinterpret_cast<uint32_t*>(INTEGER(wrap(Rmapping)));
  #pragma omp parallel for
  for(size_t i = 0;i < Rmapping.size();i++) {
    pmapping[i] = pmapping[i] % hash_size;
  }
  IntegerVector i(m.slot("i")), p(m.slot("p")), dimension(m.slot("Dim"));
  NumericVector x(m.slot("x"));
  dimension[0] = hash_size;
  #pragma omp parallel for
  for(size_t j = 0;j < i.size();j++) {
    i[j] = pmapping[i[j]];
  }
}