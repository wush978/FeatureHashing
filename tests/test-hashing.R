if (require(RUnit)) {
  library(FeatureHashing)
  
  mapping_value <- structure(c(3789462177, 4122940517, 1079927366, 1505155248, 4103768016, 
    1576910802, 248868694, 2189134401, 1321560276, 2636986885, 1980993114, 
    3588767725, 3873367263, 3437882550, 1125161513, 875000041, 1178743966, 
    1791688646), .Names = c("PlantQn1", "PlantQn2", "PlantQn3", "uptake", 
      "TypeMississippi", "Treatmentchilled", "PlantMn1", "PlantMn2", 
      "PlantMn3", "PlantQc1", "PlantQc2", "PlantQc3", "Treatmentnonchilled", 
      "PlantMc1", "PlantMc2", "PlantMc3", "conc", "TypeQuebec"))
  
  checkTrue(all(hashed.value(names(mapping_value)) %% 2^32 == mapping_value),
            "Unexpected hashing result by hashed.value")
  
  m <- hashed.model.matrix(~ ., CO2, hash.size = 2^10, create.mapping = TRUE, transpose = TRUE, is.dgCMatrix = FALSE, signed.hash = TRUE)
  mapping <- as.list(attr(m, "mapping"))
  checkTrue(all(!duplicated(unlist(mapping) %% 2^10 + 1)),
            "Unexpected collision of hashing example")
  
  name.candidate <- lapply(seq_along(CO2), function(i) {
    if (class(CO2[[i]])[1] == "numeric") rep(names(CO2)[i], nrow(CO2)) else {
      paste0(names(CO2)[i], CO2[[i]])
    }
  })
  
  for(index in seq_along(mapping)) {
    i <- hashed.value(names(mapping))[index] %% 2^10 + 1
    name <- names(mapping)[index]
    col.i <- which(sapply(name.candidate, function(x) name %in% x))
    
    X <- name.candidate[[col.i]]
    j <- which(name == X)
    value <- hash.sign(names(mapping))[index]
    if (class(CO2[[col.i]])[1] == "numeric") {
      value <- value * CO2[[col.i]]
    }
    
    x <- m[i,]
    checkTrue(all(x[j] == value), "Inconsistent hash value between hashed.value, hash.sign and hashed.model.matrix")
    checkTrue(all(x[-j] == 0), "Inconsistent hash value between hashed.value, hash.sign and hashed.model.matrix")
  }
  
  m <- hashed.model.matrix(~ .^2, CO2, hash.size = 2^10, create.mapping = TRUE,
                           transpose = TRUE, is.dgCMatrix = FALSE, signed.hash = TRUE)
  mapping_value <- hash.mapping(m)
  
  mapping_value.expected <- structure(list(PlantQc1 = 2636986885, PlantQn1 = 3789462177, 
                           PlantQc2 = 1980993114, PlantQn2 = 4122940517, "PlantMc3:conc" = 3739801583, 
                           "PlantMc2:conc" = 1395604525, "PlantMn3:conc" = 3235311896, 
                           "PlantMn2:conc" = 540436913, "PlantQc1:TypeQuebec" = 925725157, 
                           "TypeQuebec:conc" = 2746149015, "PlantQn1:TypeQuebec" = 4237186241, 
                           PlantQc3 = 3588767725, "PlantQn1:conc" = 2288578128, "Treatmentchilled:conc" = 255906650, 
                           PlantQn3 = 1079927366, "PlantQc1:conc" = 2251098928, TypeQuebec = 1791688646, 
                           "PlantMc3:Treatmentchilled" = 3891992855, "PlantMn2:Treatmentnonchilled" = 1166626593, 
                           "TypeQuebec:Treatmentchilled" = 2941354564, "PlantMn1:Treatmentnonchilled" = 3205838849, 
                           Treatmentnonchilled = 3873367263, Treatmentchilled = 1576910802, 
                           "PlantMc2:uptake" = 175028363, "PlantMc1:uptake" = 4172577474, 
                           "PlantMn3:uptake" = 4053818018, "TypeQuebec:uptake" = 3178360588, 
                           "PlantQc3:uptake" = 3937745018, uptake = 1505155248, "PlantMc3:TypeMississippi" = 1611851048, 
                           "PlantMn3:TypeMississippi" = 2164201561, PlantMc1 = 3437882550, 
                           PlantMn1 = 248868694, PlantMc2 = 1125161513, PlantMn2 = 2189134401, 
                           PlantMc3 = 875000041, PlantMn3 = 1321560276, "PlantQn3:conc" = 3023324759, 
                           "TypeMississippi:conc" = 1821057777, "PlantQc2:conc" = 714503009, 
                           "PlantQc3:conc" = 1657455447, "PlantQn2:TypeQuebec" = 2781903365, 
                           "PlantQc2:TypeQuebec" = 51079300, "PlantQn3:TypeQuebec" = 862836424, 
                           "PlantQn2:conc" = 900931883, "PlantQc3:TypeQuebec" = 510730956, 
                           "TypeMississippi:Treatmentchilled" = 2281473014, "PlantMn3:Treatmentnonchilled" = 1193828828, 
                           "PlantQc1:Treatmentchilled" = 4048766167, "PlantQc2:Treatmentchilled" = 2534699902, 
                           "PlantQn2:Treatmentnonchilled" = 1386368423, "PlantQn1:Treatmentnonchilled" = 160472416, 
                           "PlantMc3:uptake" = 712540968, "PlantQn2:uptake" = 974267571, 
                           "Treatmentnonchilled:uptake" = 48404478, "PlantQn1:uptake" = 3757368420, 
                           "conc:uptake" = 375043273, "PlantMc1:conc" = 3318696701, 
                           conc = 1178743966, "Treatmentnonchilled:conc" = 4015153694, 
                           "PlantMn1:conc" = 1646616462, "PlantMc2:Treatmentchilled" = 2148207650, 
                           "PlantMc1:Treatmentchilled" = 3227113198, "PlantQc3:Treatmentchilled" = 3565755962, 
                           "TypeQuebec:Treatmentnonchilled" = 1973117262, "TypeMississippi:Treatmentnonchilled" = 4231588217, 
                           "PlantQn3:Treatmentnonchilled" = 1411718622, "PlantMn2:uptake" = 342355816, 
                           "TypeMississippi:uptake" = 31699673, "PlantQn3:uptake" = 4247950021, 
                           "PlantQc2:uptake" = 3035200074, "Treatmentchilled:uptake" = 3235544148, 
                           "PlantQc1:uptake" = 946686067, "PlantMn1:uptake" = 3186749149, 
                           "PlantMc2:TypeMississippi" = 3984179893, "PlantMc1:TypeMississippi" = 517052919, 
                           "PlantMn2:TypeMississippi" = 2154821378, "PlantMn1:TypeMississippi" = 3688281232, 
                           TypeMississippi = 4103768016), .Names = c("PlantQc1", "PlantQn1", 
                                                                     "PlantQc2", "PlantQn2", "PlantMc3:conc", "PlantMc2:conc", "PlantMn3:conc", 
                                                                     "PlantMn2:conc", "PlantQc1:TypeQuebec", "TypeQuebec:conc", "PlantQn1:TypeQuebec", 
                                                                     "PlantQc3", "PlantQn1:conc", "Treatmentchilled:conc", "PlantQn3", 
                                                                     "PlantQc1:conc", "TypeQuebec", "PlantMc3:Treatmentchilled", "PlantMn2:Treatmentnonchilled", 
                                                                     "TypeQuebec:Treatmentchilled", "PlantMn1:Treatmentnonchilled", 
                                                                     "Treatmentnonchilled", "Treatmentchilled", "PlantMc2:uptake", 
                                                                     "PlantMc1:uptake", "PlantMn3:uptake", "TypeQuebec:uptake", "PlantQc3:uptake", 
                                                                     "uptake", "PlantMc3:TypeMississippi", "PlantMn3:TypeMississippi", 
                                                                     "PlantMc1", "PlantMn1", "PlantMc2", "PlantMn2", "PlantMc3", "PlantMn3", 
                                                                     "PlantQn3:conc", "TypeMississippi:conc", "PlantQc2:conc", "PlantQc3:conc", 
                                                                     "PlantQn2:TypeQuebec", "PlantQc2:TypeQuebec", "PlantQn3:TypeQuebec", 
                                                                     "PlantQn2:conc", "PlantQc3:TypeQuebec", "TypeMississippi:Treatmentchilled", 
                                                                     "PlantMn3:Treatmentnonchilled", "PlantQc1:Treatmentchilled", 
                                                                     "PlantQc2:Treatmentchilled", "PlantQn2:Treatmentnonchilled", 
                                                                     "PlantQn1:Treatmentnonchilled", "PlantMc3:uptake", "PlantQn2:uptake", 
                                                                     "Treatmentnonchilled:uptake", "PlantQn1:uptake", "conc:uptake", 
                                                                     "PlantMc1:conc", "conc", "Treatmentnonchilled:conc", "PlantMn1:conc", 
                                                                     "PlantMc2:Treatmentchilled", "PlantMc1:Treatmentchilled", "PlantQc3:Treatmentchilled", 
                                                                     "TypeQuebec:Treatmentnonchilled", "TypeMississippi:Treatmentnonchilled", 
                                                                     "PlantQn3:Treatmentnonchilled", "PlantMn2:uptake", "TypeMississippi:uptake", 
                                                                     "PlantQn3:uptake", "PlantQc2:uptake", "Treatmentchilled:uptake", 
                                                                     "PlantQc1:uptake", "PlantMn1:uptake", "PlantMc2:TypeMississippi", 
                                                                     "PlantMc1:TypeMississippi", "PlantMn2:TypeMississippi", "PlantMn1:TypeMississippi", 
                                                                     "TypeMississippi"))
  mapping_value.expected <- unlist(mapping_value.expected) %% 2^10 + 1
  mapping_value[names(mapping_value.expected)]
  checkTrue(isTRUE(all.equal(mapping_value[names(mapping_value.expected)], 
                             mapping_value.expected)),
            "Unexpected hashing result of interaction term")
  
  m2 <- hashed.model.matrix(~ . ^ 2, data = CO2, hash.size = 32, create.mapping = TRUE,
                            transpose = TRUE, is.dgCMatrix = FALSE, signed.hash = TRUE)
  checkTrue(!all(m2@i == 0),
            "All hashed indices created by hashed.model.matrix are zero")
  checkTrue(sum(m2 %*% rep(1, ncol(m2)) != 0) > 1,
            "Incorrect hashed matrix created by hashed.model.matrix")
  
  mapping <- hash.mapping(m2)
  mapping.raw <- hashed.value(names(mapping))
  names(mapping.raw) <- names(mapping)
  is.interaction <- grepl(":", names(mapping), fixed = TRUE)
  checkTrue(all(mapping[!is.interaction] == mapping.raw[!is.interaction] %% 32 + 1))
  checkTrue(sum(is.na(unlist(lapply(names(mapping), strsplit, "")))) == 0,
            "Non-ascii name occurs!")
  
  for(i in grep(":", names(mapping), fixed = TRUE)) {
    name <- names(mapping)[i]
    key <- strsplit(name, ":")[[1]]
    input <- unlist(lapply(mapping.raw[key], intToRaw), use.names = FALSE)
    r1 <- hashed.value(rawToChar(input))
    checkTrue(r1 %% 32 + 1== mapping[name],
              "The hashing result of interaction is not expected!")
  }
  
  # check handling of NA
  tryCatch(m <- hashed.model.matrix(~ PlAnT, CO2, 8, signed.hash = TRUE,
                                    transpose = TRUE, is.dgCMatrix = FALSE), error = function(e) {
    if (class(e)[1] != "std::invalid_argument") stop(e)
    if (conditionMessage(e) != "Failed to find the column:PlAnT") stop(e)
  })
  
  m <- hashed.model.matrix(~ Plant:Type:Treatment, CO2, create.mapping = TRUE, signed.hash = TRUE)
  map <- hash.mapping(m)
  map <- map[grepl("\\w+:\\w+:\\w+", names(map))]
  checkTrue(all(hashed.interaction.value(names(map)) %% (2^18) + 1== map),
            "The hashed.interaction.value gives inconsistent result of hashed.model.matrix")
}
