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
#'@name test.tag
NULL

#'iPinYou Real-Time Bidding Dataset for Computational Advertising Research
#'
#'This is a sample from the iPinYou dataset which
#'has a benchmark of benchmark by Zhang, Yuan, Wang, et al. (2014).
#'The data.frame named \code{imp.train} is a sample from the data of 2013-10-19 and 
#'the data.frame named \code{imp.test} is a sample from the  data of 2013-10-20.
#'
#'@format
#'The column name of the data is the description of the data in Zhang, Yuan, Wang, et al. (2014). 
#'Most of the columns should be clearly described by their column names. 
#'For the details of the dataset, please read the Zhang, Yuan, Wang, et al. (2014).
#'\itemize {
#'  \item \code{BidID}, the id of the RTB which is the unique identifier of the events.
#'  \item \code{Adid}, the advertiser id.
#'  \item \code{UserTag}, the user tags (segments) in iPinYou's proprietary audience database
#'}
#'@source \url{http://data.computational-advertising.org/}
#'@references  W. Zhang, S. Yuan, J. Wang, et al. "Real-Time Bidding
#'Benchmarking with iPinYou Dataset". In: _arXiv preprint
#'arXiv:1407.7073_ (2014).
#'@examples
#'data(ipinyou)
#'colnames(imp.train)
#'colnames(imp.test)
#'@name ipinyou
NULL