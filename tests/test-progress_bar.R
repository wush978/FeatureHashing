if (require(RUnit)) {
  library(methods)
  library(FeatureHashing)
  pd <- capture.output(m1 <- hashed.model.matrix(~ ., CO2, 2^5, transpose = TRUE, progress = TRUE))
  checkTrue(all.equal(paste(pd, collapse = ""), paste(c(
    "", 
    "0%   10   20   30   40   50   60   70   80   90   100%", 
    "|----|----|----|----|----|----|----|----|----|----|", 
    "***************************************************"
  ), collapse = "")))
}