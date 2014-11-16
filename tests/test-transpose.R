library(methods)
library(FeatureHashing)
m1 <- as(hashed.model.matrix(~ ., CO2, 2^5), "dgCMatrix")
m2 <- as(hashed.model.matrix(~ ., CO2, 2^5, F), "dgCMatrix")

m1.1 <- as(m1, "matrix")
m2.1 <- as(m2, "matrix")
stopifnot(all(t(m1.1) == m2.1))
