library(FeatureHashing)

tryCatch(m <- hashed.model.matrix(~ PlAnT, CO2, 8), error = function(e) {
  if (class(e) != "std::invalid_argument") stop(e)
  if (conditionMessage(e) != "Failed to find the column:PlAnT") stop(e)
})

