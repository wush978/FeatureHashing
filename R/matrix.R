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
setAs(.CSRMatrix, "matrix", function(from) tomatrix(from))

setMethod("[", c(.CSRMatrix, "numeric", "numeric"), 
          function(x, i, j, ..., drop = TRUE) {
            iset <- head(seq(from = x@p[j], to = x@p[j + 1], by = 1), -1) + 1
            if (i %in% x@i[iset]) {
              sum(x@x[iset[i == x@i[iset]]])
            } else 0
          })

setMethod("[", c(.CSRMatrix, "numeric", "missing"),
          function(x, i, j, ..., drop = TRUE) {
            if (drop) return(.selectRow(x, i, drop)) else {
              retval <- new(.CSRMatrix)
              .selectRow(x, i, drop, retval)
              class(retval) <- .CSRMatrix
              retval@Dimnames[[2]] <- paste(seq_len(retval@Dim[2]))
              return(retval)
            }
          })

setMethod("[", c(.CSRMatrix, "missing", "numeric"), 
          function(x, i, j, ..., drop = TRUE) {
            if (drop) return(.selectColumn(x, j, drop)) else {
              retval <- new(.CSRMatrix)
              .selectColumn(x, j, drop, retval)
              class(retval) <- .CSRMatrix
              retval@Dimnames[[2]] <- paste(seq_len(retval@Dim[2]))
              return(retval)
            }
          })

