#include <Rcpp.h>
using namespace Rcpp;

// Below is a simple example of exporting a C++ function to R. You can
// source this function into an R session using the Rcpp::sourceCpp 
// function (or via the Source button on the editor toolbar)

// For more on using Rcpp click the Help button on the editor toolbar

//[[Rcpp::export]]
NumericVector Xv(S4 m, NumericVector v, NumericVector retval) {
  IntegerVector i(m.slot("i")), p(m.slot("p")), Dim(m.slot("Dim"));
  NumericVector x(m.slot("x"));
  retval.fill(0.0);
  for(size_t col = 0;col < Dim[1];col++) {
    for(int k = p[col];k < p[col + 1];k++) {
      int row = i[k];
      double value = x[k];
      retval[row] += value * v[col];
    }
  }
  return retval;
}

//[[Rcpp::export]]
NumericVector vX(NumericVector v, S4 m, NumericVector retval) {
  IntegerVector i(m.slot("i")), p(m.slot("p")), Dim(m.slot("Dim"));
  NumericVector x(m.slot("x"));
  retval.fill(0.0);
  #pragma omp parallel for
  for(size_t col = 0;col < Dim[1];col++) {
    for(int k = p[col];k < p[col + 1];k++) {
      int row = i[k];
      double value = x[k];
      retval[col] += value * v[row];
    }
  }
  return retval;
}
