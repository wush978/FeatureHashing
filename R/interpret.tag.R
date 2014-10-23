#'@title Interpret tag in formula
#'@param object formula
#'@param data data.frame
#'@examples
#'\dontrun{
#'  data <- data.frame(a = c("1,2,3", "2,3,3", "1,3", "3"), type = c("a", "b", "a", "a"), stringsAsFactors = FALSE)
#'  r <- interpret.tag(~ tag(a, split = ",") * type, data)
#'}
#'@export
interpret.tag <- function(object, data) {
  tf <- terms.formula(object, specials = "tag", data = data)
  terms <- attr(tf, "term.labels")
  nt <- length(terms)
  tagp <- attr(tf, "specials")$tag
  vtab <- attr(tf, "factors")
  tagst <- list()
  for(i in tagp) {
    origin.keep.source <- options()$keep.source
    tryCatch({
      options(keep.source = TRUE)
      tagst[[i]] <- eval(p <- parse(text = rownames(vtab)[i]), envir = data)
    }, finally = {options(keep.source = origin.keep.source)})
    tmp <- getParseData(p)
    stopifnot(tmp[which(tmp$token == "SYMBOL"), "text"] %in% names(data))
    pre.name <- sprintf("%s_%s", tmp[which(tmp$token == "SYMBOL"), "text"], attr(tagst[[i]], "type"))
    names(tagst[[i]]) <- sprintf("%s__%s", pre.name, names(tagst[[i]]))
    for(j in seq_along(tagst[[i]])) {
      data[[names(tagst[[i]])[j]]] <- tagst[[i]][[j]]
    }
  }
  av <- character(0)
  for(i in seq_len(ncol(vtab))) {
    term <- list()
    for(j in seq_len(nrow(vtab))) {
      if (!vtab[j,i]) next
      if (j %in% tagp) {
        term <- append(term, list(names(tagst[[j]])))
      } else {
        term <- append(term, list(rownames(vtab)[j]))
      }
    }
    av <- append(av, apply(expand.grid(term), 1, paste, collapse = ":"))
  }
  if (!is.null(off <- attr(tf, "offset"))) {
    av <- append(av, as.character(attr(tf, "variables")[1 + off]))
  }
  if (attr(tf, "response") > 0) {
    response <- as.character(attr(tf, "variables")[2])
  } else {
    response <- NULL
  }
  pf <- paste(response, "~", paste(av, collapse = "+"))
  if (attr(tf, "intercept") == 0) {
    pf <- paste(pdf, "-1", sep = "")
  }
  list(object = as.formula(pf, environment(object)), data = data)
}