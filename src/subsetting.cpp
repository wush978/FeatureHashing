#include <unordered_map>
#include <Rcpp.h>
using namespace Rcpp;

//[[Rcpp::export(".selectColumn")]]
SEXP selectColumn(S4 m, IntegerVector index, bool drop = true, SEXP Rretval = R_NilValue) {
  IntegerVector i(m.slot("i")), p(m.slot("p")), Dim(m.slot("Dim"));
  NumericVector x(m.slot("x"));
  #pragma omp parallel for
  for(auto pindex = index.begin();pindex < index.end();pindex++) {
    if (*pindex <= 0 | *pindex > Dim[1]) throw std::invalid_argument("Out of range!");
  }
  switch(index.size()) {
    case 0:
      return R_NilValue;
    case 1: {
      NumericVector retval(Dim[0], 0.0);
      auto col = index[0] - 1;
      for(auto j = p[col];j < p[col + 1];j++) {
        auto row = i[j];
        double value = x[j];
        retval[row] += value;
      }
      return retval;
    }
    default: {
      if (drop) {
        NumericMatrix retval(Dim[0], index.size());
        for(auto pindex = index.begin();pindex != index.end();pindex++) {
          auto col = *pindex - 1;
          for(auto j = p[col];j < p[col + 1];j++) {
            auto row = i[j];
            double value = x[j];
            retval(row, pindex - index.begin()) += value;
          }
        }
        return retval;
      }
      else {
        std::vector<int> new_i;
        IntegerVector new_p(index.size() + 1, 0);
        std::vector<double> new_x;
        size_t new_p_index = 1;
        for(auto pindex = index.begin();pindex != index.end();pindex++) {
          auto col = *pindex - 1;
          for(auto j = p[col];j < p[col + 1];j++) {
            auto row = i[j];
            new_i.push_back(row);
            double value = x[j];
            new_x.push_back(value);
          }
          new_p[new_p_index++] = new_i.size();
        }
        S4 retval(Rretval);
        retval.slot("i") = wrap(new_i);
        retval.slot("p") = new_p;
        retval.slot("x") = wrap(new_x);
        IntegerVector dim(2);
        {
          dim[0] = Dim[0];
          dim[1] = index.size();
          retval.slot("Dim") = dim;
        }
        List dimnames(2);
        dimnames[0] = CharacterVector(0);
        dimnames[1] = CharacterVector(0);
        retval.slot("Dimnames") = dimnames;
        retval.slot("factors") = List();
        return retval;
      }
    }
  }
}

//[[Rcpp::export(".selectRow")]]
SEXP selectRow(S4 m, IntegerVector index, bool drop = true, SEXP Rretval = R_NilValue) {
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
      if (drop) {
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
      else {
        std::vector<int> new_i;
        IntegerVector new_p(p.size(), 0);
        std::vector<double> new_x;
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
            new_i.push_back(k->second);
            new_x.push_back(value);
          }
          new_p[col + 1] = new_i.size();
        }
        S4 retval(Rretval);
        retval.slot("i") = wrap(new_i);
        retval.slot("p") = new_p;
        retval.slot("x") = wrap(new_x);
        IntegerVector dim(2);
        {
          dim[0] = index.size();
          dim[1] = Dim[1];
          retval.slot("Dim") = dim;
        }
        List dimnames(2);
        dimnames[0] = CharacterVector(0);
        dimnames[1] = CharacterVector(0);
        retval.slot("Dimnames") = dimnames;
        retval.slot("factors") = List();
        return retval;
      }
    }
  }
}
