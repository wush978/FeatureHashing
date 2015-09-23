
#'@export
#'@title Register Special Function for Formula Interface
#'@param special string. The name which will be used in formula interface.
#'@param callback_generator function which will create a callback. Please see the details.
#'@examples
#'register_callback("split", generate_split_callback)
register_callback <- function(special, callback_generator) {
  .callback[[special]] <- callback
  invisible(NULL)
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
