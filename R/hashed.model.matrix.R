#'@title Create a Model Matrix with Feature Hashing
#'@param object formula. A model formula.
#'@param data data.frame. The original data.
#'@param hash_size positive integer. The hash size of feature hashing.
#'@param transpose logical value. Indicating if the transpose should be returned.
#'@param keep.hashing_mapping logical value. 
#'The indicator of whether store the hash mapping.
#'
#'@details
#'The \code{hashed.model.matrix} hashes the feature automatically during
#'the construction of the model matrix. It uses the 32-bit variant of MurmurHash3 
#'\url{https://code.google.com/p/smhasher/wiki/MurmurHash3}. Weinberger 
#'et. al. (2009) used two seperate hashing function \eqn{h} and \eqn{\xi}
#'to determine the index and the sign of a feature, respectively. Different
#'seed is used to implement the hashing function \eqn{h} and \eqn{\xi} with
#'MurmurHash3.
#'
#'@references Kilian Q. Weinberger, Anirban Dasgupta, John Langford, 
#'Alexander J. Smola, and Josh Attenberg. ICML, volume 382 of ACM 
#'International Conference Proceeding Series, page 140. ACM, (2009)
#'
#'@examples
#'# Construct the model matrix. The transposed matrix is returned by default.
#'m <- hashed.model.matrix(~ ., CO2, 8, keep.hashing_mapping = TRUE)
#'# Print the matrix via dgCMatrix
#'as(m, "dgCMatrix")
#'# Check the result of hashing
#'ls(attr(m, "mapping"))
#'# The row of "conc" is collided with the intercept if the hash size is 8
#'attr(m, "mapping")[["conc"]] %% 8
#'# The result is CSCMatrix which supports simple subsetting and matrix-vector
#'# multiplication
#'
#'@export
#'@importFrom methods new
#'@importFrom methods checkAtAssignment
hashed.model.matrix <- function(object, data, hash_size = 2^24, transpose = TRUE, keep.hashing_mapping = FALSE) {
  stopifnot(hash_size >= 0)
  stopifnot(is.data.frame(data))
  tf <- terms.formula(object, data = data, specials = "tag")
  retval <- new(.CSCMatrix)
  .hashed.model.matrix.dataframe(tf, data, hash_size, transpose, retval, keep.hashing_mapping)
  class(retval) <- .CSCMatrix
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
