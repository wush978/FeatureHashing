library(FeatureHashing)

i <- sample(1:10, 10, TRUE)
x <- rnorm(10)

FeatureHashing:::pair_sort(i, x)
i.bak <- integer(10)
x.bak <- numeric(10)
for(.i in seq_along(i)) {
  i.bak[.i] <- i[.i]
  x.bak[.i] <- x[.i]
}
i.bak
x.bak

tmp <- FeatureHashing:::merge(i, x)
stopifnot(isTRUE(all.equal(i[1:tmp], sort(unique(i.bak)))))
stopifnot(isTRUE(all.equal(x[1:tmp], local({
  retval <- sapply(split(x.bak, i.bak), sum)
  names(retval) <- NULL
  retval
  }))))
