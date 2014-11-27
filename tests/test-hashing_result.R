library(FeatureHashing)

m <- hashed.model.matrix(~ ., CO2, hash_size = 2^10, keep.hashing_mapping = T)
mapping <- as.list(attr(m, "mapping"))
stopifnot(all(!duplicated(unlist(mapping) %% 2^10 + 1)))

name.candidate <- lapply(seq_along(CO2), function(i) {
  if (class(CO2[[i]])[1] == "numeric") rep(names(CO2)[i], nrow(CO2)) else {
    paste0(names(CO2)[i], CO2[[i]])
  }
})

for(index in seq_along(mapping)) {
  i <- hash_h(names(mapping))[index] %% 2^10 + 1
  name <- names(mapping)[index]
  col.i <- which(sapply(name.candidate, function(x) name %in% x))
  
  X <- name.candidate[[col.i]]
  j <- which(name == X)
  value <- hash_xi(names(mapping))[index]
  if (class(CO2[[col.i]])[1] == "numeric") {
    value <- value * CO2[[col.i]]
  }
  
  x <- m[i,]
  stopifnot(x[j] == value)
  stopifnot(x[-j] == 0)
}
