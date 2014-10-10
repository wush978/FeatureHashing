library(Matrix)
library(FeatureHashing)
contr <- list(
  Plant = contrasts(CO2$Plant, contrasts = FALSE),
  Type = contrasts(CO2$Type, contrasts = FALSE),
  Treatment = contrasts(CO2$Treatment, contrasts = FALSE)
  )
m1 <- sparse.model.matrix(~ ., CO2, contr, transpose = TRUE)

v <- numeric(dim(m1)[2])
r1 <- as.vector(m1 %*% v)
r2 <- FeatureHashing:::Xv(m1, v, numeric(dim(m1)[1]))
stopifnot(isTRUE(all.equal(r1, r2)))

for(i in 1:ncol(m1)) {
  v <- numeric(dim(m1)[2])
  v[i] <- 1
  r1 <- as.vector(m1 %*% v)
  r2 <- numeric(dim(m1)[1])
  FeatureHashing:::Xv(m1, v, r2)
  stopifnot(isTRUE(all.equal(r1, r2)))
  r <- as.vector(m1[,i])
}


v <- numeric(dim(m1)[1])
r1 <- as.vector(v %*% m1)
r2 <- FeatureHashing:::vX(v, m1, numeric(dim(m1)[2]))
stopifnot(isTRUE(all.equal(r1, r2)))

for(i in 1:nrow(m1)) {
  v <- numeric(dim(m1)[1])
  v[i] <- 1
  r1 <- as.vector(v %*% m1)
  r2 <- numeric(dim(m1)[2])
  FeatureHashing:::vX(v, m1, r2)
  stopifnot(isTRUE(all.equal(r1, r2)))
}
