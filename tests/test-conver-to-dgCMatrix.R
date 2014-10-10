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
