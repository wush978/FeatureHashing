#'@title Create a Model Matrix with Feature Hashing
#'@param object formula. A symbolic description of the model.
#'The details of model specification are given under 'Details'.
#'@param data data.frame. The original data.
#'@param hash_size positive integer. The hash size of feature hashing.
#'@param keep.hashing.mapping logical value. 
#'The indicator of whether store the hash mapping.
#'@export
#'@importFrom methods new
#'@importFrom methods checkAtAssignment
hashed.model.matrix <- function(object, data, hash_size = 2^24, keep.hashing_mapping = FALSE) {
  stopifnot(hash_size >= 0)
  stopifnot(is.data.frame(data))
  tf <- terms.formula(object, data = data, specials = "tag")
  retval <- new(.CSRMatrix)
  .hashed.model.matrix.dataframe(tf, data, hash_size, retval, keep.hashing_mapping)
  class(retval) <- .CSRMatrix
  retval@Dimnames[[2]] <- paste(seq_len(retval@Dim[2]))
  retval
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
