#'@title Feature Hashing of Model Matrix
#'@importFrom Matrix sparse.model.matrix
#'@importClassesFrom SparseM matrix.csc
#'@importFrom SparseM as.matrix.csr
#'@export
hashed.model.matrix <- function(object, data = environment(object),
  xlev = NULL, transpose = FALSE, 
  drop.unused.levels = FALSE, row.names = TRUE, verbose = FALSE, 
  hash_size = 2^24, ...) {
  .data.class <- sapply(data, class, simplify = FALSE, USE.NAMES = TRUE)
  .data.categoric.i <- sapply(.data.class, simplify = TRUE, function(s) {
    ("character" %in% s) | ("factor" %in% s)
  })
  .data.categoric <- names(data)[.data.categoric.i]
  contrasts.arg <- sapply(.data.categoric, simplify = FALSE, USE.NAMES = TRUE,
    function(name) {
      contrasts(data[[name]], contrasts = FALSE)
  })
  m <- sparse.model.matrix(object, data, contrasts.arg, xlev, transpose,
                           drop.unused.levels, row.names, verbose, ...)
  print(colnames(m))
  mapping <- hash_without_intercept(colnames(m))
  X.csc <- new("matrix.csc", ra = m@x, ja = m@i + 1L, ia = m@p + 1L, dimension = m@Dim)
  X.csr <- as.matrix.csr(X.csc)
  hash_internal(X.csr, mapping, hash_size)
  X.csr
}
  