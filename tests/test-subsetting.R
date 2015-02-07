if (require(RUnit)) {
  library(Matrix)
  library(FeatureHashing)
  contr <- list(
    Plant = contrasts(CO2$Plant, contrasts = FALSE),
    Type = contrasts(CO2$Type, contrasts = FALSE),
    Treatment = contrasts(CO2$Treatment, contrasts = FALSE)
    )
  m1 <- sparse.model.matrix(~ ., CO2, contr, transpose = TRUE)
  m2 <- m1
  class(m2) <- "CSCMatrix"
  
  r1 <- as.vector(m1[1,])
  r2 <- FeatureHashing:::.selectRow(m1, 1)
  checkTrue(isTRUE(all.equal(r1, r2)),
            "The result of C++ .selectRow is incorrect")
  r3 <- m2[1.0, ]
  checkTrue(isTRUE(all.equal(r1, r3)),
            "The result of operator `[` is incorrect at first argument")
  
  r1 <- as.vector(m1[,1])
  r2 <- FeatureHashing:::.selectColumn(m1, 1)
  checkTrue(isTRUE(all.equal(r1, r2)),
            "The result of C++ .selectColumn is incorrect")
  r3 <- m2[,1]
  checkTrue(isTRUE(all.equal(r1, r3)),
            "The result of `[` is incorrect at second argument")
  
  for(i in 1:10) {
    j <- sample(1:nrow(m1), 1)
    r1 <- as.vector(m1[j,])
    r2 <- FeatureHashing:::.selectRow(m1, j)
    checkTrue(isTRUE(all.equal(r1, r2)),
              "The result of C++ .selectRow is incorrect")
    r3 <- m2[j,]
    checkTrue(isTRUE(all.equal(r1, r3)),
              "The result of operator `[` is incorrect at first argument")
    
    j <- sample(1:ncol(m1), 1)
    r1 <- as.vector(m1[,j])
    r2 <- FeatureHashing:::.selectColumn(m1, j)
    checkTrue(isTRUE(all.equal(r1, r2)),
              "The result of C++ .selectColumn is incorrect")
    r3 <- m2[,j]
    checkTrue(isTRUE(all.equal(r1, r3)),
              "The result of `[` is incorrect at second argument")
  }
  
  r1 <- as.matrix(m1[1:2,])
  attr(r1, "dimnames") <- NULL
  r2 <- FeatureHashing:::.selectRow(m1, 1:2)
  checkTrue(isTRUE(all.equal(r1, r2)))
  m2 <- m1
  class(m2) <- "CSCMatrix"
  r3 <- m2[1:2, drop = FALSE]
  checkTrue(isTRUE(all.equal(r1, local({
    retval <- as(as(r3, "dgCMatrix"), "matrix")
    attr(retval, "dimnames") <- NULL
    retval
  }))))
  checkTrue(isTRUE(all.equal(r1, as(r3, "matrix"))))
  r4 <- m2[1:2,]
  checkTrue(isTRUE(all.equal(r1, r4)))
  
  r1 <- as.matrix(m1[,1:2])
  attr(r1, "dimnames") <- NULL
  r2 <- FeatureHashing:::.selectColumn(m1, 1:2)
  checkTrue(isTRUE(all.equal(r1, r2)))
  r3 <- m2[, 1:2, drop = FALSE]
  checkTrue(isTRUE(all.equal(r1, local({
    retval <- as(as(r3, "dgCMatrix"), "matrix")
    attr(retval, "dimnames") <- NULL
    retval
  }))))
  checkTrue(isTRUE(all.equal(r1, as(r3, "matrix"))))
  r4 <- m2[,1:2]
  checkTrue(isTRUE(all.equal(r1, r4)))
  
  
  r1 <- as.matrix(m1[i.set <- c(1, 3, 4, 16, 9),])
  attr(r1, "dimnames") <- NULL
  r2 <- FeatureHashing:::.selectRow(m1, i.set)
  checkTrue(isTRUE(all.equal(r1, r2)))
  r3 <- m2[i.set, drop = FALSE]
  checkTrue(isTRUE(all.equal(r1, local({
    retval <- as(as(r3, "dgCMatrix"), "matrix")
    attr(retval, "dimnames") <- NULL
    retval
  }))))
  checkTrue(isTRUE(all.equal(r1, as(r3, "matrix"))))
  r4 <- m2[i.set,]
  checkTrue(isTRUE(all.equal(r1, r4)))
  
  r1 <- as.matrix(m1[, j.set <- c(1, 3, 4, 16, 9)])
  attr(r1, "dimnames") <- NULL
  r2 <- FeatureHashing:::.selectColumn(m1, j.set)
  checkTrue(isTRUE(all.equal(r1, r2)))
  r3 <- m2[, j.set, drop = FALSE]
  checkTrue(isTRUE(all.equal(r1, local({
    retval <- as(as(r3, "dgCMatrix"), "matrix")
    attr(retval, "dimnames") <- NULL
    retval
  }))))
  checkTrue(isTRUE(all.equal(r1, as(r3, "matrix"))))
  r4 <- m2[,j.set]
  checkTrue(isTRUE(all.equal(r1, r4)))
  
}
