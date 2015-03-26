#'@title Extract mapping between hash and original values
#'
#'@importFrom magrittr %>%
#'
#'@param matrix Matrix returned by \code{hashed.model.matrix} function
#'
#'@return a named \code{numeric} vector
#'
#'@details
#'Generate a mapping between original values and hashes. 
#'
#'Option \code{create.mapping = T} needs to be used in function \code{hashed.model.matrix}.
#'
#'Original values are stores in the names of the vector.
#'
#'@examples
#'data(ipinyou)
#'
#'m <- hashed.model.matrix(~., ipinyou.train, 2^10, create.mapping = TRUE)
#'mapping <- hash.mapping(m)
#'
#'@author Michael Benesty
#'@export
hash.mapping <- function(matrix) {
  ErrorMessage <- "No mapping embedded. Please use create.mapping = TRUE option with hashed.model.matrix function."
  
  if(!"mapping" %in% (matrix %>% attributes %>% names)) stop(ErrorMessage)
  mapping <- matrix %>% attr("mapping")
  if(length(mapping) == 0) stop(ErrorMessage)
  mapping %>% as.list %>% unlist(recursive = FALSE) %>% `+`(1)
}