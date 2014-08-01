#include <cstring>
#include <Rcpp.h>
#include "md5.h"
#include "crc32.h"
using namespace Rcpp;

//[[Rcpp::export(hash_without_intercept)]]
IntegerVector hash_without_intercept(CharacterVector src) {
  IntegerVector retval(src.size(), 0);
  ::MD5_CTX ctx;
  unsigned char md5buf[16];
  uint32_t crc32buf;
  for(int i = 0;i < src.size();i++) {
    const char* str = CHAR(src[i]);
    if (::strcmp("(Intercept)", str) == 0) continue;
    ::MD5Init(&ctx);
    ::MD5Update(&ctx, reinterpret_cast<const unsigned char*>(str), ::strlen(str));
    ::MD5Final(md5buf, &ctx);
    crc32buf = 0;
    retval[i] = ::Crc32_ComputeBuf(crc32buf, reinterpret_cast<void*>(md5buf), 16);
  }
  return retval;
}