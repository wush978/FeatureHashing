library(FeatureHashing)

randomString <- function(n) {
  len <- rpois(n, 10)
  sapply(len, function(l) paste(sample(letters, l, T), collapse = ""))
}

if (interactive()) n <- 10^6 else n <- 10^2
x <- randomString(n)
print("multi")
system.time(r1 <- FeatureHashing:::hash_without_intercept(x))
print("single")
system.time(r2 <- FeatureHashing:::hash_without_intercept_single(x))
stopifnot(all.equal(r1, r2))