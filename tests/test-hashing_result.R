library(FeatureHashing)

m <- hashed.model.matrix(~ ., CO2, hash_size = 2^10, keep.hashing_mapping = T)
mapping <- as.list(attr(m, "mapping"))
stopifnot(all(!duplicated(unlist(mapping) %% 2^10 + 1)))

for(index in length(mapping)) {
  i <- hash_h(names(mapping))[index] %% 2^10 + 1
  j <- which(CO2$Plant == "Qn2")
  value <- hash_xi(names(mapping))[index]
  
  x <- m[i,]
  stopifnot(x[j] == value)
  stopifnot(x[-j] == 0)
}
