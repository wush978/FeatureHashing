if (require(RUnit)) {
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
  checkTrue(all.equal(x, x.bak[order(i.bak)]),
            "The result of pair_result is incorrect")
  checkTrue(all(diff(i) >= 0),
            "The result of pair_result is incorrect")
  
  i <- sample(1:10, 10, TRUE)
  x <- rnorm(10)
  
  FeatureHashing:::pair_sort(i, x)
  i.bak <- integer(10)
  x.bak <- numeric(10)
  for(.i in seq_along(i)) {
    i.bak[.i] <- i[.i]
    x.bak[.i] <- x[.i]
  }
  
  tmp <- FeatureHashing:::merge(i, x)
  checkTrue(isTRUE(all.equal(i[1:tmp], sort(unique(i.bak)))),
            "The result of C++ pair_sort and merge is incorrect")
  checkTrue(isTRUE(all.equal(x[1:tmp], local({
    retval <- sapply(split(x.bak, i.bak), sum)
    names(retval) <- NULL
    retval
    }))), 
    "The result of C++ pair_sort and merge is incorrect")
  
  contr <- list(
    Plant = contrasts(CO2$Plant, contrasts = FALSE),
    Type = contrasts(CO2$Type, contrasts = FALSE),
    Treatment = contrasts(CO2$Treatment, contrasts = FALSE)
  )
  
  m4 <- hashed.model.matrix(~ ., data = CO2, hash.size = 2^4, 
                            transpose = TRUE, is.dgCMatrix = FALSE)
  
  m5 <- as(m4, "dgCMatrix")
  m6 <- hashed.model.matrix(~ ., data = CO2, hash.size = 2^4,
                            transpose = TRUE)
  invisible(capture.output(print(m5)))
  stopifnot(isTRUE(all.equal(dim(m4), dim(m5))))
  stopifnot(isTRUE(all.equal(sum(m4@x), sum(m5@x))))
  stopifnot(isTRUE(all.equal(dim(m4), dim(m6))))
  stopifnot(isTRUE(all.equal(sum(m4@x), sum(m6@x))))
  for(i in seq_len(dim(m4)[2])) {
    e1 <- numeric(dim(m4)[2])
    e1[i] <- 1.0
    r4 <- m4 %*% e1
    r5 <- as.vector(m5 %*% e1)
    r6 <- as.vector(m6 %*% e1)
    stopifnot(isTRUE(all.equal(r4, r5)))
    stopifnot(isTRUE(all.equal(r4, r6)))
  }
  
  for(i in seq_len(dim(m4)[1])) {
    e1 <- numeric(dim(m4)[1])
    e1[i] <- 1.0
    r4 <- e1 %*% m4
    r5 <- as.vector(e1 %*% m5)
    r6 <- as.vector(e1 %*% m6)
    stopifnot(isTRUE(all.equal(r4, r5)))
    stopifnot(isTRUE(all.equal(r4, r6)))
  }
}
