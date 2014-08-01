#include <cstring>
#include <Rcpp.h>
#include "capi.h"
using namespace Rcpp;

//[[Rcpp::export("hash_without_intercept")]]
IntegerVector hash_without_intercept(CharacterVector src) {
  IntegerVector retval(src.size(), 0);
  char md5buf[16];
  for(int i = 0;i < src.size();i++) {
    const char* str = CHAR(src[i]);
    if (::strcmp("(Intercept)", str) == 0) continue;
    md5(str, strlen(str), md5buf);
    retval[i] = crc32(md5buf, 16);
  }
  return retval;
}

//[[Rcpp::export("rehash_inplace")]]
SEXP rehash_inplace(S4 m, IntegerVector Rmapping, int hash_size) {
  uint32_t *pmapping = reinterpret_cast<uint32_t*>(INTEGER(wrap(Rmapping)));
  for(int i = 0;i < Rmapping.size();i++) {
    pmapping[i] = pmapping[i] % hash_size;
  }
  IntegerVector ja(m.slot("ja"));
  for(int i = 0;i < ja.size();i++) {
    ja[i] = pmapping[ja[i] - 1] + 1;
  }
  IntegerVector dimension(m.slot("dimension"));
  dimension[1] = hash_size;
  return R_NilValue;
}