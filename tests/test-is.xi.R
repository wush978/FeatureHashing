if (require(RUnit)) {
  library(FeatureHashing)
  m1 <- hashed.model.matrix(~ ., iris, hash_size = 64, is.xi = TRUE)
  m2 <- hashed.model.matrix(~ ., iris, hash_size = 64, is.xi = FALSE)
  checkTrue(Reduce(`|`, as.matrix(m1) < 0))
  for(j in 1:64) {
    m1.sign <- sign(sum(m1[,j]))
    checkEqualsNumeric(m1.sign * m1[,j], m2[,j])
  }
}