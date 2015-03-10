if (require(RUnit)) {
  library(FeatureHashing)
  data(test.tag)
  system.time(tmp1 <- FeatureHashing:::split.character1(test.tag, ",", type = "existence"))
  system.time(tmp2 <- FeatureHashing:::split.character2(test.tag, ",", type = "existence"))
  checkTrue(isTRUE(all.equal(tmp1, tmp2)),
            "The split results (type existence) between R and C++ are inconsistent")
  
  system.time(tmp1 <- FeatureHashing:::split.character1(test.tag, ",", type = "count"))
  system.time(tmp2 <- FeatureHashing:::split.character2(test.tag, ",", type = "count"))
  checkTrue(isTRUE(all.equal(tmp1, tmp2)),
            "The split results (type count) between R and C++ are inconsistent")
}
