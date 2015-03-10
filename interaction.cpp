//[[Rcpp::depends(digest)]]

#include <Rcpp.h>
#include <pmurhashAPI.h>
using namespace Rcpp;

//[[Rcpp::export]]
int interaction(int a, int b) {
  char buf[8];
  memcpy(buf, &a, 4);
  memcpy(buf + 4, &b, 4);
  return (int) PMurHash32(3120602769LL, buf, 8);
}