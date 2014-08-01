hash_internal <- function(m, ...) {
  UseMethod("hash_internal")
}

#'@importClassesFrom Matrix dgCMatrix
hash_internal.matrix.csr <- function(m, mapping, hash_size) {
  rehash_inplace(m, mapping, hash_size);
}