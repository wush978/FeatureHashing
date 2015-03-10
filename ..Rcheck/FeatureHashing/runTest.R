library(RUnit)
test <- defineTestSuite("FeatureHashing", dirs = "tests", testFileRegexp = "^test-")
runTestSuite(test)
