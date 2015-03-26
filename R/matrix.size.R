#'@title Compute minimum hash size to reduce collision rate
#'
#'@importFrom magrittr %>%
#'
#'@param df \code{data.frame} with data to hash
#'
#'@return The hash size of feature hashing as a positive integer.
#'
#'@details
#'To reduce collision rate, the hash size should be 
#'equal or superior to the nearest power of two to the number
#'of unique values in the input \code{data.frame}.
#'
#'The value computed is a theorical minimum hash size.
#'It just means that in the best situation it may be 
#'possible that all computed hash can be stored with
#'this hash size.
#'
#'In real life, there will be some collisions if the computed
#'size is used. It is because of the way hashing works.
#'
#'If you increase the computed size (by multiplying it by \code{2^x}, 
#'it is up to you to choose a \code{x}), you should reduce the collision rate.
#'If you use a value under the computed size,
#'tehre is 100% change there will be some collisions.
#'
#'There is a trade-off between collision rate and memory
#'used to store hash. Machine learning algorithms usually deal
#'well with collisions when the rate is reasonable.
#'
#'@examples
#'data(ipinyou)
#'
#'#First try with a size of 2^10
#'mat1 <- hashed.model.matrix(~., ipinyou.train, 2^10, create.mapping = TRUE)
#'
#'#Extract mapping
#'mapping1 <- hash.mapping(mat1)
#'#Rate of collision
#'mean(duplicated(mapping1 %% 2^10))
#'
#'#Second try, the size is computed
#'size <- hash.size(ipinyou.train)
#'mat2 <- hashed.model.matrix(~., ipinyou.train, size, create.mapping = TRUE)
#'
#'#Extract mapping
#'mapping2 <- hash.mapping(mat2)
#'#Rate of collision
#'mean(duplicated(mapping2 %% size))
#'
#'@export
hash.size <- function(df) {
   sapply(df, function(x) unique(x) %>% length) %>% sum %>% log2 %>% ceiling %>% 2^.
}

# Avoid error messages during CRAN check.
# The reason is that these variables are never declared
# They are mainly column names inferred by Data.table...
globalVariables(c("."))