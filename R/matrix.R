#'@title CSCMatrix
#'@description The structure of \code{CSCMatrix} is the same
#'as the structure of \code{dgCMatrix}. However, the
#'\code{CSCMatrix} has weaker constraints compared to \code{dgCMatrix}.
#'
#'\code{CSCMatrix} onlysupports limited operators. The users can convert it to
#'\code{dgCMatrix} for compatibility of existed algorithms.
#'@seealso \code{\link{dgCMatrix-class}}
#'@details The \code{CSCMatrix} violates two constraints used in \code{dgCMatrix}:
#'\itemize{
#'  \item The row indices should be sorted with columns.
#'  \item The row indices should be unique with columns.
#'}
#'The result of matrix-vector multiplication should be the same.
#'@examples
#'# construct a CSCMatrix
#'m <- hashed.model.matrix(~ ., CO2, 8)
#'# convert it to dgCMatrix
#'m2 <- as(m, "dgCMatrix")
#'@aliases dim<-,CSCMatrix-method dim,CSCMatrix-method 
#'%*%,CSCMatrix,numeric-method 
#'%*%,numeric,CSCMatrix-method 
#'[,CSCMatrix,missing,numeric,ANY-method
#'[,CSCMatrix,numeric,missing,ANY-method 
#'[,CSCMatrix,numeric,numeric,ANY-method
#'@section Methods:
#'\itemize{
#'\item \code{dim} The dimension of the matrix object \code{CSCMatrix}.
#'\item \code{dim<-} The assignment of dimension of the matrix object \code{CSCMatrix}.
#'\item \code{[} The subsetting operator of the matrix object \code{CSCMatrix}.
#'\item \code{\%*\%} The matrix-vector multiplication of the matrix object \code{CSCMatrix}.
#'The returned object is a numeric vector.
#'}
setClass("CSCMatrix", representation(
  i = "integer",
  p = "integer",
  Dim = "integer",
  Dimnames = "list",
  x = "numeric",
  factors = "list"))

.CSCMatrix <- "CSCMatrix"
attr(.CSCMatrix, "package") <- .packageName

setMethod("dim", signature(x = .CSCMatrix), function(x) x@Dim)

setMethod("dim<-", signature(x = .CSCMatrix, value = "ANY"), function(x, value) {
  x@Dim <- value
})

setMethod("%*%", signature(x = .CSCMatrix, y = "numeric"), function(x, y) {
  stopifnot(x@Dim[2] == length(y))
  Xv(x, y, numeric(x@Dim[1]))
})

setMethod("%*%", signature(x = "numeric", y = .CSCMatrix), function(x, y) {
  stopifnot(y@Dim[1] == length(x))
  vX(x, y, numeric(y@Dim[2]))
})

setAs(.CSCMatrix, "dgCMatrix", function(from) todgCMatrix(from))
setAs(.CSCMatrix, "matrix", function(from) tomatrix(from))

setMethod("[", c(.CSCMatrix, "numeric", "numeric"), 
          function(x, i, j, ..., drop = TRUE) {
            iset <- head(seq(from = x@p[j], to = x@p[j + 1], by = 1), -1) + 1
            if (i %in% x@i[iset]) {
              sum(x@x[iset[i == x@i[iset]]])
            } else 0
          })

setMethod("[", c(.CSCMatrix, "numeric", "missing"),
          function(x, i, j, ..., drop = TRUE) {
            if (drop) return(.selectRow(x, i, drop)) else {
              retval <- new(.CSCMatrix)
              .selectRow(x, i, drop, retval)
              class(retval) <- .CSCMatrix
              retval@Dimnames[[2]] <- paste(seq_len(retval@Dim[2]))
              return(retval)
            }
          })

setMethod("[", c(.CSCMatrix, "missing", "numeric"), 
          function(x, i, j, ..., drop = TRUE) {
            if (drop) return(.selectColumn(x, j, drop)) else {
              retval <- new(.CSCMatrix)
              .selectColumn(x, j, drop, retval)
              class(retval) <- .CSCMatrix
              retval@Dimnames[[2]] <- paste(seq_len(retval@Dim[2]))
              return(retval)
            }
          })

