if (require(RUnit)) {
  library(methods)
  library(FeatureHashing)
  m1 <- hashed.model.matrix(~ ., CO2, 2^5, transpose = TRUE)
  m2 <- hashed.model.matrix(~ ., CO2, 2^5, transpose = FALSE)

  m1.1 <- as(m1, "matrix")
  m2.1 <- as(m2, "matrix")
  checkTrue(all(t(m1.1) == m2.1),
            "The transpose argument produces incorrect result")
}
