#'@title Expand concatenated feature
#'@param x character vector or factor. The source of tag features.
#'@param split character vector. The split symbol for tag features.
#'@param type character value. Either "count" or "existence". "count" indicates the number of occurrence of the tag. "existence" indicates the boolean that whether the tag exist or not.
#'@return integer vector for \code{type = "count"} and logical vector for \code{type = "existence"}.
#'@export
tag <- function(x, split = ",", type = c("count", "existence")) {
  retval <- switch(class(x), 
    "character" = tag.character2(x, split, type),
    "factor" = tag.factor(x, split, type),
  )
  attr(retval, "type") <- type[1]
  retval
}

tag.character1 <- function(x, split, type) {
  x <- strsplit(x, split = split)
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

tag.character2 <- function(x, split, type) {
  switch(type[1], 
      "count" = tag_count(x, split),
      "existence" = tag_existence(x, split)
      )
}

tag.factor <- function(x, split, type) {
  retval.levels <- tag.character1(levels(x), split, type)
  retval.i <- as.integer(x)
  retval <- list()
  for(i in seq_along(retval.levels)) {
    name <- names(retval.levels)[i]
    retval[[name]] <- retval.levels[[i]][retval.i]
  }
  retval
}
