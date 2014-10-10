library(FeatureHashing)

i <- sample(1:10, 10, TRUE)
x <- rnorm(10)

i.bak <- integer(10)
x.bak <- numeric(10)
for(.i in seq_along(i)) {
  i.bak[.i] <- i[.i]
  x.bak[.i] <- x[.i]
}
i.bak
FeatureHashing:::pair_sort(i, x)
stopifnot(all.equal(x, x.bak[order(i.bak)]))
stopifnot(all(diff(i) >= 0))
