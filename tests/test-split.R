if (require(RUnit)) {
  library(FeatureHashing)
  df <- data.frame(b = c("1,2", "1,,3", ",2,3"), stringsAsFactors = FALSE)
  m <- hashed.model.matrix(~ split(b), df, create.mapping = TRUE)
  mapping <- hash.mapping(m)
  checkTrue(all.equal(sort(names(mapping)), paste("b", 1:3, sep="")), "the default split is incorrect")
  m <- hashed.model.matrix(~ split(b, delim = "2"), df, create.mapping = TRUE)
  mapping <- hash.mapping(m)
  checkTrue(all.equal(sort(names(mapping)),
                      sort(paste("b", unique(unlist(strsplit(df$b, "2", fixed = TRUE))), sep=""))
  ), "the delim of split is incorrect")
  m <- hashed.model.matrix(~ split(b, delim = ",", type = "count"), df, create.mapping = TRUE)
  mapping <- hash.mapping(m)
  checkTrue(all.equal(sort(names(mapping)), paste("b", 1:3, sep="")), "the default split is incorrect")
  for(key in names(mapping)) {
    pattern <- substring(key, 2, nchar(key))
    checkTrue(all.equal(sum(grepl(pattern, df$b, fixed=TRUE)), sum(abs(m[,mapping[[key]]]))),
              "The value is inconsistent")
  }
}
