#include <unordered_map>
#include <Rcpp.h>
using namespace Rcpp;

//[[Rcpp::export(".selectRow")]]
SEXP selectRow(S4 m, IntegerVector index) {
  IntegerVector i(m.slot("i")), p(m.slot("p")), Dim(m.slot("Dim"));
  NumericVector x(m.slot("x"));
  switch(i.size()) {
    case 0:
      return R_NilValue;
    case 1: {
      NumericVector retval(Dim[1], 0.0);
      #pragma omp parallel for
      for(auto col = 0;col < Dim[1];col++) {
        for(auto j = i[col];j < i[col + 1];j++) {
          auto row = i[j];
          if (row != index[0]) continue;
          double value = p[j];
          retval[col] += value;
        }
      }
      return retval;
    }
    default: {
      NumericMatrix retval(i.size(), Dim[1]);
      std::unordered_map<int, size_t> mapping;
      for(auto j = 0;j < index.size();j++) {
        mapping[index[j]]  = j;
      }
      for(auto col = 0;col < Dim[1];col++) {
        for(auto j = i[col];j < i[col + 1];j++) {
          auto row = i[j];
          auto k = mapping.find(row);
          if (k == mapping.end()) continue;
          double value = p[j];
          retval(k->second, col) += value;
        }
      }
      return retval;
    }
  }
}
