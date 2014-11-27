library(pack)
library(FeatureHashing)

m <- hashed.model.matrix(~ .^2, CO2, hash_size = 2^10, keep.hashing_mapping = T)
mapping <- as.list(attr(m, "mapping"))

# no non-ascii name
stopifnot(sum(is.na(unlist(lapply(names(mapping), strsplit, "")))) == 0)

for(i in grep(":", names(mapping), fixed = TRUE)) {
  name <- names(mapping)[i]
  key <- strsplit(name, ":")[[1]]
  input <- unlist(lapply(mapping[key], function(j) {
    numToRaw(j, 4)
  }), use.names = FALSE)
  r1 <- hash_h(rawToChar(input))
  r2 <- mapping[[name]]
  if (r1 < 0) {
    r2 <- packBits(rawToBits(numToRaw(r2, 4)), type = "integer")
  }
  stopifnot(r1 == r2)
}
