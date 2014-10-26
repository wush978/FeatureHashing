library(FeatureHashing)
m2 <- hashed.model.matrix(~ ., data = CO2)
m3 <- hashed.model.matrix(~ ., data = CO2, hash_size = 8, keep.hashing_mapping = TRUE)
ls(attr(m3, "mapping"))
