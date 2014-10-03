library(Matrix)
library(FeatureHashing)
contr <- list(
  Plant = contrasts(CO2$Plant, contrasts = FALSE),
  Type = contrasts(CO2$Type, contrasts = FALSE),
  Treatment = contrasts(CO2$Treatment, contrasts = FALSE)
  )
m1 <- sparse.model.matrix(~ ., CO2, contr, transpose = TRUE)
m2 <- hashed.model.matrix(~ ., data = CO2, contr)
m3 <- hashed.model.matrix(~ ., data = CO2, contr, hash_size = NULL)
stopifnot(isTRUE(all.equal(m1, m3)))

