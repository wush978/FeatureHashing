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
  cat(sprintf("original feature size is %d\n", nrow(m)))
  if (is.null(hash_size)) return(m)
  mapping <- hash_without_intercept(rownames(m))
  cat(sprintf("estimated collision rate is %0.8f\n", sum(duplicated(mapping)) / length(mapping)))
  rehash_inplace(m, mapping, hash_size)
  class(m) <- .CSRMatrix
  m@Dim[1] <- as.integer(hash_size)
  m@Dimnames[[1]] <- character(0)
  m
}

#'@importFrom methods new
hashed.model.matrix2 <- function(object, data = environment(object), hash_size = 2^24, keep.hashing_mapping = FALSE) {
  tf <- terms.formula(object, data = data)
  retval <- new(.CSRMatrix)
  .hashed.model.matrix(tf, data, hash_size, retval)
  class(retval) <- .CSRMatrix
  retval@Dimnames[[2]] <- paste(seq_len(retval@Dim[2]))
  retval
}