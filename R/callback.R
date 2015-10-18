
#'@export
#'@title Register Special Function for Formula Interface
#'@param special string. The name which will be used in formula interface.
#'@param callback_generator function which will create a callback. Please see the details.
#'@details The callback_generator is a function whose first argument is the
#'input data and the other arguments could be used to initialize the callback
#'function properly. The result should be a Rcpp module which derives the
#'`CallbackFunctor` class. Please see the vignette for details.
#'register_callback("split", generate_split_callback)
register_callback <- function(special, callback_generator) {
  .callback[[special]] <- callback_generator
  invisible(NULL)
}

#'@title List the Registered Specials
#'@return character vector. The specials which could be used in the 
#'formula interface.
#'@export
ls_special <- function() {
  ls(.callback)
}

#'@title Generate callback of split
#'@param input character vector. The input of split
#'@param delim string. \code{delim} will be used as delimiter for splitting
#'@param type string. One of \code{c("existence", "count")}
#'"count" indicates the number of occurrence of the token. "existence" indicates the boolean that whether the token exist or not.
#'@export
generate_split_callback <- function(input, delim = ",", type = c("existence", "count")) {
  callback <- new(split_callback, input, delim, type[1])
  callback
}

.callback <- new.env()
.callback[["split"]] <- generate_split_callback

#'@title Initialize and register jiebaR to the formula interface
#'@details This function will register the callback of word segmentation
#'function provided by jiebaR to the formula interface. 
#'For example, `~ jiebaR(...)` will use the feature of word segmentation
#'provided by jiebaR to segment a given column of the data.
#'The first argument of the jiebaR is a character which will be segmented.
#'The left arguments are the same as \code{\link[jiebaR]{worker}}. These
#'arguments will be used to initialize a jiebaR worker which will segment
#'the input data.
#'
#'@examples
#'\dontrun{
#'library(FeatureHashing)
#'init_jiebaR_callback()
#'m <- hashed.model.matrix(~ jiebaR(title, type = "mix", df))
#'# the column `df$title` will be feed into `worker <- worker(type = "mix")`
#'# the result of `worker <= df$title` will be hashed into the sparse matrix
#'# the result is `m`
#'}
#'@export
#'@importFrom Rcpp sourceCpp
init_jiebaR_callback <- function() {
  if (!requireNamespace("jiebaR")) stop("Please install the package jiebaR first")
  tryCatch({
    sourceCpp(system.file("callback/jiebaR_callback.cpp", package = "FeatureHashing"))
  }, finally = {
  })
}
