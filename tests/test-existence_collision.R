library(FeatureHashing)
df <- data.frame(a = c("a,b,c", "a,c", "b,c", "a"))
m <- hashed.model.matrix(~ split(a, delim = ",", type = "existence") - 1, df, 
                         hash.size = 4, signed.hash = FALSE, create.mapping = TRUE)
hash.mapping(m)
stopifnot(m@x == 1)
