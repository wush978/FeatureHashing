library(Matrix)
library(FeatureHashing)
contr <- list(
  Plant = contrasts(CO2$Plant, contrasts = FALSE),
  Type = contrasts(CO2$Type, contrasts = FALSE),
  Treatment = contrasts(CO2$Treatment, contrasts = FALSE)
)

m4 <- hashed.model.matrix(~ ., data = CO2, contr, hash_size = 2^4)

m5 <- as(m4, "dgCMatrix")
invisible(capture.output(print(m5)))
stopifnot(isTRUE(all.equal(dim(m4), dim(m5))))
stopifnot(isTRUE(all.equal(sum(m4@x), sum(m5@x))))
r1 <- m4 %*% (v <- rnorm(dim(m4)[2]))
r2 <- as.vector(m5 %*% v)
stopifnot(isTRUE(all.equal(r1, r2)))
