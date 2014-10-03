hash_internal <- function(m, ...) {
  UseMethod("hash_internal")
}

#'@importClassesFrom Matrix dgCMatrix
hash_internal.dgCMatrix <- function(m, mapping, hash_size) {
  rehash_inplace(m, mapping, hash_size);
}