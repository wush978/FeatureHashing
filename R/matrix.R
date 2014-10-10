setClass("CSRMatrix", representation(
  i = "integer",
  p = "integer",
  Dim = "integer",
  Dimnames = "list",
  x = "numeric",
  factors = "list"))

.CSRMatrix <- "CSRMatrix"
attr(.CSRMatrix, "package") <- .packageName

setMethod("dim", signature(x = .CSRMatrix), function(x) x@Dim)

setMethod("dim<-", signature(x = .CSRMatrix, value = "ANY"), function(x, value) {
  x@Dim <- value
})

setMethod("%*%", signature(x = .CSRMatrix, y = "numeric"), function(x, y) {
  stopifnot(x@Dim[2] == length(y))
  Xv(x, y, numeric(x@Dim[1]))
})

setMethod("%*%", signature(x = "numeric", y = .CSRMatrix), function(x, y) {
  stopifnot(y@Dim[1] == length(x))
  vX(x, y, numeric(y@Dim[2]))
})

setAs(.CSRMatrix, "dgCMatrix", function(from) todgCMatrix(from))

setMethod("[", signature(x = .CSRMatrix, i = "index", j = "missing", drop = "logical"), 
          function(x, i, j, ..., drop) .selectRow(x, i))