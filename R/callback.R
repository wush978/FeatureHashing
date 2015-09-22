
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

#'@export
generate_split_callback <- function(input, delim = ",", type = c("existence", "count")) {
  callback <- new(split_callback, input, delim, type[1])
  callback
}

.callback <- new.env()
.callback[["split"]] <- generate_split_callback
