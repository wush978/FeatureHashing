if (require(RUnit)) {
  library(FeatureHashing)
  data(test.tag)
  set.seed(1)
  df <- data.frame(a = test.tag, b = rnorm(length(test.tag)))
  m <- hashed.model.matrix(~ split(a, delim = ",", type = "existence"):b, df, 2^6,
    create.mapping = TRUE, transpose = TRUE, is.dgCMatrix = FALSE)
  checkEquals(digest::digest(m@i), "b1990ec1a8c79130c497ab2c68909e97")
  checkEquals(digest::digest(m@p), "1f9c7075abe8f7d15f924f37797ecf4e")
  checkEquals(digest::digest(m@x), "bfd54eddd6b65f22f99c06d58e00ab9a")
}
