library(FeatureHashing)
data(test.tag)
system.time(tmp1 <- FeatureHashing:::tag.character1(test.tag, ",", type = "existence"))
system.time(tmp2 <- FeatureHashing:::tag.character2(test.tag, ",", type = "existence"))
stopifnot(isTRUE(all.equal(tmp1, tmp2)))

system.time(tmp1 <- FeatureHashing:::tag.character1(test.tag, ",", type = "count"))
system.time(tmp2 <- FeatureHashing:::tag.character2(test.tag, ",", type = "count"))
stopifnot(isTRUE(all.equal(tmp1, tmp2)))
