library(RUnit)
library(FeatureHashing)
data(test.tag)
df <- data.frame(a = test.tag, b = rnorm(length(test.tag)))
debug(hashed.model.matrix)
m <- hashed.model.matrix(~ tag(a, split = ",", type = "existence"):b, df, 2^6,
  keep.hashing_mapping = TRUE)
