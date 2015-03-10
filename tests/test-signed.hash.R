if (require(RUnit)) {
  library(FeatureHashing)
  m1 <- hashed.model.matrix(~ ., iris, hash.size = 64, signed.hash = TRUE)
  m2 <- hashed.model.matrix(~ ., iris, hash.size = 64, signed.hash = FALSE)
  checkTrue(Reduce(`|`, as.matrix(m1) < 0))
  for(j in 1:64) {
    m1.sign <- sign(sum(m1[,j]))
    checkEqualsNumeric(m1.sign * m1[,j], m2[,j])
  }
}