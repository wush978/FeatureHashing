#include <Rcpp.h>
using namespace Rcpp;

void pair_sort(int* i, double* x, size_t n) {
  if (n == 0) return;
  std::vector<size_t> ref(n, 0);
  for(size_t j = 0;j < n;j++) ref[j] = j;
  std::sort(ref.begin(), ref.end(), [&](size_t a, size_t b) { return i[a] < i[b]; });
  std::vector<int> buf_i(n, 0);
  std::vector<double> buf_x(n, 0);
  for(size_t j = 0;j < n;j++) {
    buf_i[j] = i[ref[j]];
    buf_x[j] = x[ref[j]];
  }
  for(size_t j = 0;j < n;j++) {
    i[j] = buf_i[j];
    x[j] = buf_x[j];
  }
}

//[[Rcpp::export("pair_sort")]]
void pair_sort(IntegerVector i, NumericVector x) {
  int *pi = &i[0];
  double *px = &x[0];
  pair_sort(pi, px, i.size());
}

size_t merge(int *i, double *x, size_t n) {
  if (n == 0) return 0;
  size_t len = 0, current_i = i[0];
  for(size_t j = 0;j < n;j++) {
    if (i[j] < current_i) continue;
    current_i = i[j];
    i[len] = i[j];
    size_t k = 1;
    x[len] = x[j];
    while(j + k < n) {
//      Rprintf("j:%d k:%d current_i:%d i[j + k]:%d\n", j, k, current_i, i[j + k]);
      if (current_i == i[j + k]) {
//        Rprintf("x[%d] += x[%d] => %f\n", len, j + k, x[len] + x[j + k]);
        x[len] += x[j + k];
      } else break;
      k++;
    }
    len++;
    current_i++;
  }
  return len;
}

//[[Rcpp::export]]
int merge(IntegerVector i, NumericVector x) {
  int *pi = &i[0];
  double *px = &x[0];
  return merge(pi, px, i.size());
}

// [[Rcpp::export]]
SEXP todgCMatrix(S4 m) {
  S4 retval("dgCMatrix");
  {
    CharacterVector class_name(retval.attr("class"));
    class_name.attr("package") = wrap("Matrix");
  }
  std::vector<std::string> slot_names(as< std::vector<std::string> >(Function("slotNames")(m)));
  // copy slots
  for(auto pname = slot_names.begin();pname != slot_names.end();pname++) {
    retval.slot(*pname) = m.slot(*pname);
  }
  IntegerVector i(m.slot("i")), p(m.slot("p")), Dim(m.slot("Dim"));
  NumericVector x(m.slot("x"));
  int *pi = &i[0];
  double *px = &x[0];
  std::vector<int> new_i, new_p;
  std::vector<double> new_x;
  new_i.resize(i.size());
  new_p.resize(p.size());
  new_x.resize(x.size());
  int *pnew_i = &new_i[0];
  double *pnew_x = &new_x[0];
  size_t len = 0;
  for(auto col = 0;col < Dim[1];col++) {
    new_p[col] = len;
    auto src_len = p[col + 1] - p[col];
    memcpy(pnew_i + len, pi + p[col], sizeof(int) * src_len);
    memcpy(pnew_x + len, px + p[col], sizeof(double) * src_len);
    pair_sort(pnew_i + len, pnew_x + len, src_len);
    len += merge(pnew_i + len, pnew_x + len, src_len);
  }
  new_p[Dim[1]] = len;
  new_i.resize(len);
  new_x.resize(len);
  retval.slot("i") = wrap(new_i);
  retval.slot("p") = wrap(new_p);
  retval.slot("x") = wrap(new_x);
  return retval;
}

//[[Rcpp::export]]
SEXP tomatrix(S4 m) {
  IntegerVector i(m.slot("i")), p(m.slot("p")), Dim(m.slot("Dim"));
  NumericVector x(m.slot("x"));
  NumericMatrix retval(Dim[0], Dim[1]);
  retval.fill(0.0);
  for(auto col = 0;col < Dim[1];col++) {
    for(auto j = p[col];j < p[col + 1];j++) {
      auto row = i[j];
      auto value = x[j];
      retval(row, col) += value;
    }
  }
  return retval;
}
