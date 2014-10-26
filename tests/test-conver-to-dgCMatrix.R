library(Matrix)
library(FeatureHashing)
contr <- list(
  Plant = contrasts(CO2$Plant, contrasts = FALSE),
  Type = contrasts(CO2$Type, contrasts = FALSE),
  Treatment = contrasts(CO2$Treatment, contrasts = FALSE)
)

m4 <- hashed.model.matrix(~ ., data = CO2, hash_size = 2^4)

m5 <- as(m4, "dgCMatrix")
invisible(capture.output(print(m5)))
stopifnot(isTRUE(all.equal(dim(m4), dim(m5))))
stopifnot(isTRUE(all.equal(sum(m4@x), sum(m5@x))))
for(i in seq_len(dim(m4)[2])) {
  e1 <- numeric(dim(m4)[2])
  e1[i] <- 1.0
  r4 <- m4 %*% e1
  r5 <- as.vector(m5 %*% e1)
  stopifnot(isTRUE(all.equal(r4, r5)))
}

for(i in seq_len(dim(m4)[1])) {
  e1 <- numeric(dim(m4)[1])
  e1[i] <- 1.0
  r4 <- e1 %*% m4
  r5 <- as.vector(e1 %*% m5)
  stopifnot(isTRUE(all.equal(r4, r5)))
}

