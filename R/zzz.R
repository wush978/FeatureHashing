#'@useDynLib FeatureHashing
#'@importFrom Rcpp evalCpp
#'@import digest
.onLoad <- function(libname, pkgname) { }

.onAttach <- function(libname, pkgname) {
  if (interactive()) {
    packageStartupMessage(
"The default behavior of the hashed.model.matrix is changed:
    - The default output is converted to dgCMatrix
    - The default value of `transpose` is FALSE
"
)
  }
}

#'@title test.tag
#'@description This is a vector to demo the concatenated feature.
#'@format For each element, the string represents the occurrence
#' of different tags. For example, the string "1,27,19,25,tp,tw"
#' of the first instance represents that the feature `1` is TRUE, the feature `27` is 
#' TRUE, et. al. On the contrary, the missing feature such as `2` 
#' is FALSE.
test.tag <- NULL
