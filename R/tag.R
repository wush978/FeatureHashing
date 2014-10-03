tag <- function(x, split = ",", type = c("count", "existence")) {
  switch(class(x), 
    "character" = tag.character(x, split, type),
    "factor" = tag.factor(x, split, type),
  )
}

tag.character <- function(x, split, type) {
  x <- strsplit(x, split = ",")
  x.levels <- sort(unique(unlist(x)))
  retval <- list()
  for(x.element in x.levels) {
    retval[[x.element]] <- switch(type[1], 
      "count" = sapply(x, function(s) sum(x.element == s)),
      "existence" = sapply(x, function(s) x.element %in% s)
      )
  }
  retval
}

tag.factor <- function(x, split, type) {
  retval.levels <- tag.character(levels(x), split, type)
  retval.i <- as.integer(x)
  retval <- list()
  for(i in seq_along(retval.levels)) {
    name <- names(retval.levels)[i]
    retval[[name]] <- retval.levels[[i]][retval.i]
  }
  retval
}
