# library(Matrix)
library(FeatureHashing)
m1 <- Matrix::sparse.model.matrix(~ ., CO2, contrasts.arg = list(
  Plant = contrasts(CO2$Plant, contrasts = FALSE),
  Type = contrasts(CO2$Type, contrasts = FALSE),
  Treatment = contrasts(CO2$Treatment, contrasts = FALSE)
  ))
m2 <- hashed.model.matrix(~ ., CO2)

# v1 <- rnorm(dim(m1)[2])
# v2 <- rep(0, dim(m2)[2])
# v2[attr(m2, "mapping") + 1] <- v1
# stopifnot(isTRUE(all.equal(as.vector(m1 %*% v1), as.vector(m2 %*% v2))))
