pkgname <- "FeatureHashing"
source(file.path(R.home("share"), "R", "examples-header.R"))
options(warn = 1)
base::assign(".ExTimings", "FeatureHashing-Ex.timings", pos = 'CheckExEnv')
base::cat("name\tuser\tsystem\telapsed\n", file=base::get(".ExTimings", pos = 'CheckExEnv'))
base::assign(".format_ptime",
function(x) {
  if(!is.na(x[4L])) x[1L] <- x[1L] + x[4L]
  if(!is.na(x[5L])) x[2L] <- x[2L] + x[5L]
  options(OutDec = '.')
  format(x[1L:3L], digits = 7L)
},
pos = 'CheckExEnv')

### * </HEADER>
library('FeatureHashing')

base::assign(".oldSearch", base::search(), pos = 'CheckExEnv')
cleanEx()
nameEx("CSCMatrix-class")
### * CSCMatrix-class

flush(stderr()); flush(stdout())

base::assign(".ptime", proc.time(), pos = "CheckExEnv")
### Name: CSCMatrix-class
### Title: CSCMatrix
### Aliases: CSCMatrix-class [,CSCMatrix,missing,numeric,ANY-method
###   [,CSCMatrix,numeric,missing,ANY-method
###   [,CSCMatrix,numeric,numeric,ANY-method %*%,CSCMatrix,numeric-method
###   %*%,numeric,CSCMatrix-method dim,CSCMatrix-method
###   dim<-,CSCMatrix-method

### ** Examples

# construct a CSCMatrix
m <- hashed.model.matrix(~ ., CO2, 8)
# convert it to dgCMatrix
m2 <- as(m, "dgCMatrix")



base::assign(".dptime", (proc.time() - get(".ptime", pos = "CheckExEnv")), pos = "CheckExEnv")
base::cat("CSCMatrix-class", base::get(".format_ptime", pos = 'CheckExEnv')(get(".dptime", pos = "CheckExEnv")), "\n", file=base::get(".ExTimings", pos = 'CheckExEnv'), append=TRUE, sep="\t")
cleanEx()
nameEx("hashed.model.matrix")
### * hashed.model.matrix

flush(stderr()); flush(stdout())

base::assign(".ptime", proc.time(), pos = "CheckExEnv")
### Name: hashed.model.matrix
### Title: Create a model matrix with feature hashing
### Aliases: hash_h hash_xi hashed.model.matrix

### ** Examples

# Construct the model matrix. The transposed matrix is returned by default.
m <- hashed.model.matrix(~ ., CO2, 2^6, keep.hashing_mapping = TRUE)
# Print the matrix via dgCMatrix
as(m, "dgCMatrix")
# Check the result of hashing
mapping <- unlist(as.list(attr(m, "mapping")))
# Check the rate of collision
# mean(duplicated(mapping %% 2^6))
# The result is CSCMatrix which supports simple subsetting and matrix-vector
# multiplication
# rnorm(2^6) %*% m

# Detail of the hashing
## The main effect is hashed via `hash_h`
all(hash_h(names(mapping)) %% 2^6 == mapping %% 2^6)
## The sign is corrected by `hash_xi`
hash_xi(names(mapping))
## The interaction term is implemented as follow:
m2 <- hashed.model.matrix(~ .^2, CO2, 2^6, keep.hashing_mapping = TRUE)
mapping2 <- unlist(as.list(attr(m2, "mapping")))
mapping2[2] # PlantQn2:uptake
h1 <- mapping2["PlantQn2"]
h2 <- mapping2["uptake"]
library(pack)
hash_h(rawToChar(c(numToRaw(h1, 4), numToRaw(h2, 4)))) # should be mapping2[2]

# The tag-like feature
data(test.tag)
df <- data.frame(a = test.tag, b = rnorm(length(test.tag)))
m <- hashed.model.matrix(~ tag(a, split = ",", type = "existence"):b, df, 2^6,
 keep.hashing_mapping = TRUE)
# The column `a` is splitted by "," and have an interaction with "b":
mapping <- unlist(as.list(attr(m, "mapping")))
names(mapping)



base::assign(".dptime", (proc.time() - get(".ptime", pos = "CheckExEnv")), pos = "CheckExEnv")
base::cat("hashed.model.matrix", base::get(".format_ptime", pos = 'CheckExEnv')(get(".dptime", pos = "CheckExEnv")), "\n", file=base::get(".ExTimings", pos = 'CheckExEnv'), append=TRUE, sep="\t")
### * <FOOTER>
###
options(digits = 7L)
base::cat("Time elapsed: ", proc.time() - base::get("ptime", pos = 'CheckExEnv'),"\n")
grDevices::dev.off()
###
### Local variables: ***
### mode: outline-minor ***
### outline-regexp: "\\(> \\)?### [*]+" ***
### End: ***
quit('no')
