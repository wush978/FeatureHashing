if (require(RUnit)) {
  library(FeatureHashing)
  data(test.tag)
  df <- data.frame(a = test.tag, b = rnorm(length(test.tag)))
  debug(hashed.model.matrix)
  m <- hashed.model.matrix(~ split(a, delim = ",", type = "existence"):b, df, 2^6,
    create.mapping = TRUE, transpose = TRUE, is.dgCMatrix = FALSE)
}
