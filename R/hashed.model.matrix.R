#'@title Feature Hashing of Model Matrix
#'@importFrom Matrix sparse.model.matrix
#'@export
hashed.model.matrix <- function(object, data = environment(object),
  constrasts.arg = NULL, xlev = NULL, transpose = FALSE, 
  drop.unused.levels = FALSE, row.names = TRUE, verbose = FALSE, 
  hash_size = 2^24, ...) {
  
}
  