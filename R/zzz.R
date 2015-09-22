#'@useDynLib FeatureHashing
#'@importFrom Rcpp evalCpp
#'@import digest
.onLoad <- function(libname, pkgname) { }

.onAttach <- function(libname, pkgname) {
  if (interactive()) {
    packageStartupMessage(
"
The default behavior of the hashed.model.matrix is changed:
    - The `signed.hash` is disabled.
    - The output of `split` with type `\"existence\"` will filter the collision.
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

#'@title iPinYou Real-Time Bidding Dataset for Computational Advertising Research
#'
#'@description
#'This is a sample from the iPinYou Real-Time Bidding dataset.
#'The data.frame named \code{ipinyou.train} is a sample from the data of 2013-10-19 and 
#'the data.frame named \code{ipinyou.test} is a sample from the  data of 2013-10-20.
#'
#'@format
#'The column name of the data is the description of the data in Zhang, Yuan, Wang, et al. (2014). 
#'Most of the columns should be clearly described by their column names. 
#'For the details of the dataset, please read the Zhang, Yuan, Wang, et al. (2014).
#'
#'\code{BidID}, the id of the RTB which is the unique identifier of the events.
#'
#'\code{Adid}, the advertiser id.
#'
#'\code{UserTag}, the user tags (segments) in iPinYou's proprietary audience database. 
#'This is also a real example of the concatenated feature.
#'
#'@usage
#'data(ipinyou)
#'@source \url{http://data.computational-advertising.org/}
#'@references  W. Zhang, S. Yuan, J. Wang, et al. 
#'"Real-Time Bidding Benchmarking with iPinYou Dataset". 
#'In: arXiv preprint arXiv:1407.7073 (2014).
#'@name ipinyou
#'@aliases ipinyou.train ipinyou.test
NULL