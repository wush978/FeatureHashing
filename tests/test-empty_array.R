if (require(RUnit)) {
  library(FeatureHashing)
  df <- data.frame(a = rnorm(3), b = c("1,2", "1,,3", ",2,3"), stringsAsFactors = FALSE)
  m <- hashed.model.matrix(~ a * split(b), df, create.mapping = TRUE)
  mapping <- attr(m, "mapping")
  checkTrue(!"b" %in% ls(mapping), "the output contains the column corresponding to empty string")
  df$b <- factor(df$b)
  m <- hashed.model.matrix(~ a * split(b), df, create.mapping = TRUE)
  mapping <- attr(m, "mapping")
  checkTrue(!"b" %in% ls(mapping), "the output contains the column corresponding to empty string")
}
