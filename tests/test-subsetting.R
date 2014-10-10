library(Matrix)
library(FeatureHashing)
contr <- list(
  Plant = contrasts(CO2$Plant, contrasts = FALSE),
  Type = contrasts(CO2$Type, contrasts = FALSE),
  Treatment = contrasts(CO2$Treatment, contrasts = FALSE)
  )
m1 <- sparse.model.matrix(~ ., CO2, contr, transpose = TRUE)

r1 <- as.vector(m1[1,])
r2 <- FeatureHashing:::.selectRow(m1, 1)
stopifnot(isTRUE(all.equal(r1, r2)))

for(i in 1:10) {
  j <- sample(1:nrow(m1), 1)
  r1 <- as.vector(m1[j,])
  r2 <- FeatureHashing:::.selectRow(m1, j)
  stopifnot(isTRUE(all.equal(r1, r2)))
}

r1 <- as.matrix(m1[1:2,])
attr(r1, "dimnames") <- NULL
r2 <- FeatureHashing:::.selectRow(m1, 1:2)
stopifnot(isTRUE(all.equal(r1, r2)))

function() {
  system.time(r1 <- m1[1,])
  system.time(r2 <- FeatureHashing:::.selectRow(m1, 1))
}