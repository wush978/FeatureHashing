#'@title Create a model matrix with feature hashing
#'@param object formula. A model formula.
#'@param data data.frame. The original data.
#'@param hash_size positive integer. The hash size of feature hashing.
#'@param transpose logical value. Indicating if the transpose should be returned.
#'@param keep.hashing_mapping logical value. The indicator of whether storing the hash mapping or not.
#'@param is.dgCMatrix logical value. Indicating if the result is \code{dgCMatrix} or \code{CSCMatrix}
#'
#'
#'@details
#'The \code{hashed.model.matrix} hashes the feature automatically during
#'the construction of the model matrix. It uses the 32-bit variant of MurmurHash3 
#'\url{https://code.google.com/p/smhasher/wiki/MurmurHash3}. Weinberger 
#'et. al. (2009) used two separate hashing function \eqn{h}(\code{hash_h}) and 
#'\eqn{\xi}(\code{hash_xi}) to determine the indices and the sign of the values
#'respectively. Different seeds are used to implement the hashing function 
#'\eqn{h} and \eqn{\xi} with MurmurHash3.
#'
#'The object formula is parsed via \code{\link{terms.formula}} with "tag" as special
#'keyword. The interaction term is hashed in different ways. Please see example for 
#'the detailed implementation. The "tag" is used to expand the concatenated feature
#'such as "1,27,19,25,tp,tw" which represents the occurrence of multiple categorical
#'variable. The \code{hashed.model.matrix} will expand the tag feature and produce
#'the related model matrix.
#'
#'The "tag" accepts two parameters:
#'\itemize{
#'  \item \code{split}, character value used for splitting.
#'  \item \code{type}, one of \code{existence} or \code{count}.
#'}
#'The user could explore the behavior via function \code{\link{tag}}.
#'
#'@references Kilian Q. Weinberger, Anirban Dasgupta, John Langford, 
#'Alexander J. Smola, and Josh Attenberg. ICML, volume 382 of ACM 
#'International Conference Proceeding Series, page 140. ACM, (2009)
#'
#'@examples
#'# Construct the model matrix. The transposed matrix is returned by default.
#'# Below the original values will be project in a space of 2^6 dimensions
#'m <- hashed.model.matrix(~ ., CO2, 2^6, keep.hashing_mapping = TRUE, 
#'  transpose = TRUE, is.dgCMatrix = FALSE)
#'  
#'# Print the matrix via dgCMatrix
#'as(m, "dgCMatrix")
#'
#'# Extraction of the dictionary: values with their hash
#'mapping <- unlist(as.list(attr(m, "mapping")))
#'
#'# To check the rate of collisions, we will extract the indices of the hash
#'# values through the modulo-division method, count how many duplicates 
#'# we have (in best case it should be zero) and perform a mean.
#'mean(duplicated(mapping %% 2^6))
#'
#'# The type of the result produced by the function `hashed.model.matrix` 
#'# is a CSCMatrix. It supports simple subsetting and matrix-vector multiplication
#'rnorm(2^6) %*% m
#'
#'# Detail of the hashing
#'# To hash one specific value, we can use the `hash_h` function
#'# Below we will apply this function to the feature names
#'vectHash <- hash_h(names(mapping))
#'
#'# Now we will check that the result is the same than the one got with 
#'# the more generation `hashed.model.matrix` function.
#'# We will use the Modulo-division method (that's the [%% 2^6] below) to find the address in hash table easily.
#'all(vectHash %% 2^6 == mapping %% 2^6)
#'
#'# The sign is corrected by `hash_xi`
#'hash_xi(names(mapping))
#'
#'## The interaction term is implemented as follow:
#'m2 <- hashed.model.matrix(~ .^2, CO2, 2^6, keep.hashing_mapping = TRUE, 
#'  transpose = TRUE, is.dgCMatrix = FALSE)
#'# The ^ operator indicates crossing to the specified degree. 
#'# For example (a+b+c)^2 is identical to (a+b+c)*(a+b+c) 
#'# which in turn expands to a formula containing the main effects
#'# for a, b and c together with their second-order interactions. 
#'      
#'# Extract the mapping
#'mapping2 <- unlist(as.list(attr(m2, "mapping")))
#'
#'# Get the hash of combination of two items, PlantQn2 and uptake 
#'mapping2[54] # PlantQn2:uptake hash is 974267571
#'
#'# Extract hash of each item
#'h1 <- mapping2["PlantQn2"]
#'h2 <- mapping2["uptake"]
#'
#'# Computation of hash of both items combined
#'library(pack) # convert values from / to raw format
#'hash_h(rawToChar(c(numToRaw(h1, 4), numToRaw(h2, 4)))) # should be 974267571 == mapping2[54]
#'
#'# The tag-like feature
#'data(test.tag)
#'df <- data.frame(a = test.tag, b = rnorm(length(test.tag)))
#'m <- hashed.model.matrix(~ tag(a, split = ",", type = "existence"):b, df, 2^6,
#'  keep.hashing_mapping = TRUE)
#'# The column `a` is splitted by "," and have an interaction with "b":
#'mapping <- unlist(as.list(attr(m, "mapping")))
#'names(mapping)
#'
#'@export
#'@importFrom methods new
#'@importFrom methods checkAtAssignment
#'@importClassesFrom Matrix dgCMatrix
#'@aliases hash_h hash_xi
hashed.model.matrix <- function(object, data, hash_size = 2^24, transpose = FALSE, keep.hashing_mapping = FALSE, is.dgCMatrix = TRUE) {
  stopifnot(hash_size >= 0)
  stopifnot(is.data.frame(data))
  tf <- terms.formula(object, data = data, specials = "tag")
  retval <- new(.CSCMatrix)
  .hashed.model.matrix.dataframe(tf, data, hash_size, transpose, retval, keep.hashing_mapping)
  class(retval) <- .CSCMatrix
  retval@Dimnames[[2]] <- paste(seq_len(retval@Dim[2]))
  if (is.dgCMatrix) {
    retval2 <- as(retval, "dgCMatrix") 
    attributes(retval2) <- attributes(retval)
    retval2
  } else retval
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
