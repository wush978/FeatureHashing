#include <Rcpp.h>
#include <boost/predef/other/endian.h>
#include <boost/endian/conversion.hpp>

#ifndef BOOST_ENDIAN_BIG_BYTE
  #error No BOOST_ENDIAN_BIG_BYTE
#endif

using namespace Rcpp;

//'@title Convert the integer to raw vector with endian correction
//'@param src integer value.
//'@return raw vector with length 4
//'@export
//[[Rcpp::export]]
SEXP intToRaw(int src) {
  RawVector retval(4);
  uint32_t *p = (uint32_t*) &retval[0];
  #if BOOST_ENDIAN_BIG_BYTE && !BOOST_ENDIAN_LITTLE_BYTE
  p[0] = boost::endian::endian_reverse(src);
  #elif !BOOST_ENDIAN_BIG_BYTE && BOOST_ENDIAN_LITTLE_BYTE
  p[0] = src;
  #else
  #error Unknown endianness
  #endif
  return retval;
}