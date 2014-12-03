library(FeatureHashing)

# test consistency of hashing

mapping_value <- structure(c(3789462177, 4122940517, 1079927366, 1505155248, 4103768016, 
  1576910802, 248868694, 2189134401, 1321560276, 2636986885, 1980993114, 
  3588767725, 3873367263, 3437882550, 1125161513, 875000041, 1178743966, 
  1791688646), .Names = c("PlantQn1", "PlantQn2", "PlantQn3", "uptake", 
    "TypeMississippi", "Treatmentchilled", "PlantMn1", "PlantMn2", 
    "PlantMn3", "PlantQc1", "PlantQc2", "PlantQc3", "Treatmentnonchilled", 
    "PlantMc1", "PlantMc2", "PlantMc3", "conc", "TypeQuebec"))

stopifnot(all(hash_h(names(mapping_value)) %% 2^32 == mapping_value))

# test hashing result

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
