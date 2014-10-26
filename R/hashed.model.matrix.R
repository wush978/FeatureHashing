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
  if (verbose) cat(sprintf("original feature size is %d\n", nrow(m)))
  if (is.null(hash_size)) return(m)
  mapping <- hash_without_intercept(rownames(m))
  if (verbose) cat(sprintf("estimated collision rate is %0.8f\n", sum(duplicated(mapping)) / length(mapping)))
  rehash_inplace(m, mapping, hash_size)
  class(m) <- .CSRMatrix
  m@Dim[1] <- as.integer(hash_size)
  m@Dimnames[[1]] <- character(0)
  m
}

parse_tag <- function(text) {
  origin.keep.source <- options()$keep.source
  tryCatch({
    options(keep.source = TRUE)
    p <- parse(text = text)
    tmp <- getParseData(p)
    reference_name <- tmp$text[which(tmp$token == "SYMBOL")]
    if ("split" %in% tmp$text) {
      split <- tmp$text[which(tmp$text == "split")[1] + 2]
      split <- gsub(pattern = '"', replacement = '', split)
    } else {
      split <- ","
    }
    if ("type" %in% tmp$text) {
      type <- tmp$text[which(tmp$text == "type")[1] + 2]
      type <- gsub(pattern = '"', replacement = '', type)
    } else {
      type <- "existence"
    }
    list(reference_name = reference_name, split = split, type = type)
  }, finally = {options(keep.source = origin.keep.source)})
}

#'@importFrom methods new
#'@importFrom methods checkAtAssignment
#'@export
hashed.model.matrix2 <- function(object, data = environment(object), hash_size = 2^24, keep.hashing_mapping = FALSE) {
  tf <- terms.formula(object, data = data, specials = "tag")
  retval <- new(.CSRMatrix)
  .hashed.model.matrix(tf, data, hash_size, retval, keep.hashing_mapping)
  class(retval) <- .CSRMatrix
  retval@Dimnames[[2]] <- paste(seq_len(retval@Dim[2]))
  retval
}