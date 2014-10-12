#include <cblas.h>
#include <Rcpp.h>
using namespace Rcpp;
 
// [[Rcpp::export]]
SEXP dgemv(NumericMatrix A, NumericVector y, NumericVector retval, double alpha = 1.0, double beta = 0.0, bool is_A_transpose = false) {
  if ((is_A_transpose ? A.nrow() : A.ncol()) != y.size()) throw std::invalid_argument("");
  if ((is_A_transpose ? A.ncol() : A.nrow()) != retval.size()) throw std::invalid_argument("");
  double *pA = REAL(wrap(A)), *py = REAL(wrap(y)), *pretval = REAL(wrap(retval));
  cblas_dgemv(CblasColMajor, (is_A_transpose ? CblasTrans : CblasNoTrans), A.nrow(), A.ncol(), alpha, pA, A.nrow(), py, 1, beta, pretval, 1);
  return retval;
}
 
// [[Rcpp::export]]
SEXP dgemm(NumericMatrix A, NumericMatrix B, NumericMatrix C, double alpha = 1.0, double beta = 0.0, bool is_A_transpose = false, bool is_B_transpose = false) {
  int 
    A_row = (is_A_transpose ? A.ncol() : A.nrow()),
    A_col = (is_A_transpose ? A.nrow() : A.ncol()),
    B_row = (is_B_transpose ? B.ncol() : B.nrow()),
    B_col = (is_B_transpose ? B.nrow() : B.ncol()),
    C_row = C.nrow(), C_col = C.ncol();
  if (A_row != C_row) throw std::invalid_argument("");
  if (A_col != B_row) throw std::invalid_argument("");
  if (B_col != C_col) throw std::invalid_argument("");
  double *pA = REAL(wrap(A)), *pB = REAL(wrap(B)), *pC = REAL(wrap(C));
  cblas_dgemm(CblasColMajor, (is_A_transpose ? CblasTrans : CblasNoTrans), (is_B_transpose ? CblasTrans : CblasNoTrans),
    C_row, C_col, A_col, alpha, pA, (is_A_transpose ? A_col : A_row ), pB, (is_B_transpose ? B_col : B_row ), 
    beta, pC, C_row);
  return C;
}
 