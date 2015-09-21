
.callback <- new.env()

#'@export
#'@title Register Special Function for Formula Interface
#'@param special string. The name which will be used in formula interface.
#'@param callback external pointer. The pointer to the callback function. Please see the details.
register_callback <- function(special, callback) {
  .callback[[special]] <- callback
  invisible(NULL)
}

#'@export
generate_split_callback <- function(delim, type = c("existence", "count")) {
  callback <- new(split_callback, delim, type[1])
  callback
}
