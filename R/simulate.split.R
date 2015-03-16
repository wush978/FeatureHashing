#'@title Simulate how \code{split} work in \code{hashed.model.matrix} to split the string into 
#'tokens
#'@param x character vector or factor. The source of concatenated feature.
#'@param delim character value. The string to use for splitting.
#'@param type character value. Either "\code{count}" or "\code{existence}". 
#'"\code{count}" indicates the number of occurrence of the token. 
#'"\code{existence}" indicates the boolean that whether the token exist or not.
#'@usage
#'simulate.split(x, delim = ",", type = c("existence", "count"))
#'@return integer vector for \code{type = "count"} and logical vector for \code{type = "existence"}.
simulate.split <- function(x, delim = ",", type = c("existence", "count")) {
  retval <- switch(class(x), 
    "character" = split.character2(x, delim, type),
    "factor" = split.factor(x, delim, type),
  )
  attr(retval, "type") <- type[1]
  retval
}

split.character1 <- function(x, delim, type) {
  x <- strsplit(x, split = delim)
  x.levels <- sort(unique(unlist(x)))
  x.levels <- setdiff(x.levels, "")
  retval <- list()
  for(x.element in x.levels) {
    retval[[x.element]] <- switch(type[1], 
      "count" = sapply(x, function(s) sum(x.element == s)),
      "existence" = sapply(x, function(s) x.element %in% s)
      )
  }
  retval
}

split.character2 <- function(x, delim, type) {
  switch(type[1], 
      "count" = split_count(x, delim),
      "existence" = split_existence(x, delim)
      )
}

split.factor <- function(x, delim, type) {
  retval.levels <- split.character1(levels(x), delim, type)
  retval.i <- as.integer(x)
  retval <- list()
  for(i in seq_along(retval.levels)) {
    name <- names(retval.levels)[i]
    retval[[name]] <- retval.levels[[i]][retval.i]
  }
  retval
}
