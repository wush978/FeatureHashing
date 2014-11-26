library(FeatureHashing)

m <- hashed.model.matrix(~ .^2, CO2, hash_size = 2^10, keep.hashing_mapping = T)
mapping <- as.list(attr(m, "mapping"))

# no non-ascii name
stopifnot(sum(is.na(unlist(lapply(names(mapping), strsplit, "")))) == 0)
