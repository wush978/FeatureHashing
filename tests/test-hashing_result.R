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

# verify the result of interaction
m <- hashed.model.matrix(~ .^2, CO2, hash_size = 2^10, keep.hashing_mapping = TRUE)
mapping_value <- as.list(attr(m, "mapping"))

all.equal(mapping_value, 
  structure(
    list(
      conc = 1178743966, "PlantQn2:uptake" = 974267571, 
      "PlantMc2:uptake" = 175028363, PlantQn1 = 3789462177, PlantQn2 = 4122940517, 
      TypeQuebec = 1791688646, PlantQn3 = 1079927366, PlantQc1 = 2636986885, 
      PlantMn1 = 248868694, "PlantQc2:uptake" = 3035200074, PlantQc2 = 1980993114, 
      PlantMn2 = 2189134401, PlantQc3 = 3588767725, PlantMn3 = 1321560276, 
      PlantMc1 = 3437882550, TypeMississippi = 4103768016, PlantMc2 = 1125161513, 
      PlantMc3 = 875000041, "PlantMn1:uptake" = 3186749149, "PlantMn3:uptake" = 4053818018, 
      uptake = 1505155248, "PlantQn1:uptake" = 3757368420, "PlantMc1:uptake" = 4172577474, 
      "PlantQn3:uptake" = 4247950021, "PlantMc3:uptake" = 712540968, 
      Treatmentnonchilled = 3873367263, Treatmentchilled = 1576910802, 
      "PlantQc1:uptake" = 946686067, "conc:uptake" = 375043273, 
      "PlantQc3:uptake" = 3937745018, "PlantMn2:uptake" = 342355816), 
    .Names = c(
      "conc", 
      "PlantQn2:uptake", "PlantMc2:uptake", "PlantQn1", "PlantQn2", 
      "TypeQuebec", "PlantQn3", "PlantQc1", "PlantMn1", "PlantQc2:uptake", 
      "PlantQc2", "PlantMn2", "PlantQc3", "PlantMn3", "PlantMc1", "TypeMississippi", 
      "PlantMc2", "PlantMc3", "PlantMn1:uptake", "PlantMn3:uptake", 
      "uptake", "PlantQn1:uptake", "PlantMc1:uptake", "PlantQn3:uptake", 
      "PlantMc3:uptake", "Treatmentnonchilled", "Treatmentchilled", 
      "PlantQc1:uptake", "conc:uptake", "PlantQc3:uptake", "PlantMn2:uptake"
    )))
