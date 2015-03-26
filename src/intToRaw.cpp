#include <Rcpp.h>
#include <boost/detail/endian.hpp>

#ifdef linux
#include <byteswap.h>
#endif

#ifndef bswap_32
#ifdef __APPLE__
#include <libkern/OSByteOrder.h>
#else
uint32_t bswap_32(uint32_t x);
#endif
#endif

using namespace Rcpp;

//'@title Convert the integer to raw vector with endian correction
//'@param src integer value.
//'@return raw vector with length 4
//'@export
//[[Rcpp::export]]
SEXP intToRaw(int src) {
  #ifdef BOOST_BIG_ENDIAN
  bswap_32(src);
  #endif
  RawVector retval(sizeof(int));
  memcpy(&retval[0], &src, sizeof(int));
  return retval;
}