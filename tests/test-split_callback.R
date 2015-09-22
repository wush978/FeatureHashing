if (require(RUnit)) {
  library(FeatureHashing)
  callback <- generate_split_callback(letters, ",", "existence")
  checkEquals(callback$delim, ",")
  checkEquals(callback$type, "existence")
  checkEquals(test_callback(callback, "a,b,a,c,d"), letters[1:4])
  
  checkException(callback <- generate_split_callback(letters, ",", "ex"))

  callback <- generate_split_callback(letters, ",,", "existence")
  checkEquals(test_callback(callback, "a,b,a,,c,d"), c("a,b,a", "c,d"))
  
  callback <- generate_split_callback(letters, ",", "count")
  checkEquals(callback$type, "count")
  checkEquals(test_callback(callback, x <- "a,b,a,c,d"), strsplit(x, ",")[[1]])
}