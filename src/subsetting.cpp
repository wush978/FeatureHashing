#include <unordered_map>
#include <Rcpp.h>
using namespace Rcpp;

//[[Rcpp::export(".selectRow")]]
SEXP selectRow(S4 m, IntegerVector index) {
  IntegerVector i(m.slot("i")), p(m.slot("p")), Dim(m.slot("Dim"));
  NumericVector x(m.slot("x"));
  switch(index.size()) {
    case 0:
      return R_NilValue;
    case 1: {
      NumericVector retval(Dim[1], 0.0);
      #pragma omp parallel for
      for(auto col = 0;col < Dim[1];col++) {
        for(auto j = p[col];j < p[col + 1];j++) {
          auto row = i[j] + 1;
          if (row != index[0]) continue;
          double value = x[j];
          retval[col] += value;
        }
      }
      return retval;
    }
    default: {
      NumericMatrix retval(index.size(), Dim[1]);
      std::unordered_map<int, size_t> mapping;
      for(auto j = 0;j < index.size();j++) {
        mapping[index[j]]  = j;
      }
      for(auto col = 0;col < Dim[1];col++) {
        for(auto j = p[col];j < p[col + 1];j++) {
          auto row = i[j] + 1;
          auto k = mapping.find(row);
          if (k == mapping.end()) continue;
          double value = x[j];
          retval(k->second, col) += value;
        }
      }
      return retval;
    }
  }
}
