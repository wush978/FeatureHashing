#'@title Feature Hashing of Model Matrix
#'@param hash_size integer value. The size of hashed variables.
#'@inheritParams Matrix::sparse.model.matrix
#'@seealso \code{\link{sparse.model.matrix}}
#'@importFrom Matrix sparse.model.matrix
#'@export
hashed.model.matrix <- function(object, data = environment(object),
  contrasts.arg = NULL, xlev = NULL, transpose = TRUE, 
  drop.unused.levels = FALSE, row.names = TRUE, verbose = FALSE, 
  hash_size = 2^24, ...) {
  m <- sparse.model.matrix(object, data, contrasts.arg, xlev, transpose,
                           drop.unused.levels, row.names, verbose)
  if (is.null(hash_size)) return(m)
  mapping <- hash_without_intercept(rownames(m))
  rehash_inplace(m, mapping, hash_size)
  class(m) <- .CSRMatrix
  m@Dim[1] <- as.integer(hash_size)
  m@Dimnames[[1]] <- character(0)
  m
}

