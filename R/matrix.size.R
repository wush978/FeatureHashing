#'@title Compute matrix size to avoid collision
#'
#'@importFrom magrittr %>%
#'
#'@param df \code{data.frame} with data to hash
#'
#'@return The hash size of feature hashing as a positive integer.
#'
#'@details
#'To avoid collision, the hash size should be equal or superior to 
#'the nearest power of two to the number of unique values in 
#'the \code{data.frame}.
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
#'mean(duplicated(mapping1 %% 2^5))
#'
#'#Second try, the size is computed
#'size <- matrix.size(ipinyou.train)
#'mat2 <- hashed.model.matrix(~., ipinyou.train, size, create.mapping = TRUE)
#'
#'#Extract mapping
#'mapping2 <- hash.mapping(mat2)
#'#Rate of collision
#'mean(duplicated(mapping2 %% size))
#'
#'@export
matrix.size <- function(matrix) {
   sapply(matrix, function(x) unique(x) %>% length) %>% sum %>% log2 %>% ceiling %>% 2^.
}