#'@title Create a model matrix with feature hashing
#'
#'@importFrom magrittr %>%
#'@importFrom magrittr %<>%
#'@importFrom methods new
#'@importFrom methods checkAtAssignment
#'@importFrom Matrix colSums
#'@importFrom Matrix Diagonal
#'
#'@importClassesFrom Matrix dgCMatrix
#'
#'@param formula \code{formula} or a \code{character} vector of column names (will be expanded to a \code{formula})
#'@param data data.frame. The original data.
#'@param hash.size positive integer. The hash size of feature hashing.
#'@param transpose logical value. Indicating if the transpose should be returned. It affects the space
#'of the returned object when the dimension is imbalanced. Please see the details.
#'@param create.mapping logical value. The indicator of whether storing the hash mapping or not. 
#'The mapping might miss some interaction terms which involves \code{split}ed features. 
#'Please see the details.
#'@param is.dgCMatrix logical value. Indicating if the result is \code{dgCMatrix} or \code{CSCMatrix}
#'@param signed.hash logical value. Indicating if the hashed value is multipled by random sign.
#'This will reduce the impact of collision. Disable it will enhance the speed.
#'@param progress logical value. Indicating if the progress bar is displayed or not.
#'
#'@details
#'The \code{hashed.model.matrix} hashes the feature during
#'the construction of the model matrix. It uses the 32-bit variant of MurmurHash3 
#'\url{https://code.google.com/p/smhasher/wiki/MurmurHash3}. Weinberger 
#'et. al. (2009) used two separate hashing function \eqn{h}(\code{hashed.value}) and 
#'\eqn{\xi}(\code{hash.sign}) to determine the indices and the sign of the values
#'respectively. Different seeds are used to implement the hashing function 
#'\eqn{h} and \eqn{\xi} with MurmurHash3.
#'
#'The formula is parsed via \code{\link{terms.formula}} with "split" as special
#'keyword. The interaction term is hashed (the reader can try  to expl)in different ways. Please see example for 
#'the detailed implementation. We provide a helper function: \code{\link{hashed.interaction.value}} to show show the index after interaction.
#'The "\code{split}" is used to expand the concatenated feature
#'such as "10129,10024,13866,10111,10146,10120,10115,10063" which represents the occurrence of 
#'multiple categorical variable: "10129", "10024", "13866", "10111", "10146", "10120", "10115", and
#'"10063". The \code{hashed.model.matrix} will expand the concatenated feature and produce
#'the related model matrix.
#'
#'The "\code{split}" accepts two parameters:
#'\itemize{
#'  \item \code{delim}, character value to use as delimiter for splitting;
#'  \item \code{type}, one of \code{existence}, \code{count} or \code{tf-idf}.
#'}
#'
#'If \code{type} is set to \code{tf-idf}, then \code{signed.hash} should be set to \code{FALSE}.
#'
#'The user could explore the behavior via function \code{\link{simulate.split}}.
#'
#'The argument \code{transpose} affects the size of the returned object in the following way.
#'For a \eqn{m \times n} matrix with \eqn{k} non-zero elements, the returned \code{dgCMatrix} requires
#'\eqn{O(n) + O(k)} space. For details, please check the documentation of 
#'the \code{\link{dgCMatrix-class}}. Note that the \code{rownames} of the returned \code{dgCMatrix}
#'is \code{character(0)} so the space complexity does not contain the term \eqn{O(m)}.
#'
#'The \code{mapping} created by enabling \code{create.mapping} might miss the interaction term which
#'involves \code{split}ed features. For example, suppose there are two columns \code{a} and \code{b}
#'while the value are 1 and 1,2,3 respectively. The user marks the column \code{b} with 
#'\code{split}. If the hashed value of \code{b1} and \code{b2} are collided, then the interaction 
#'\code{a1:b1} will not appear in the returned mapping table. Because this package is originally 
#'designed for predictive analysis and the mapping should not play an 
#'important role of predictive analysis. If you have a test case and want to ask us to fix this, 
#'please provide us a test case in \url{https://github.com/wush978/FeatureHashing/issues/67}.
#'
#'@references 
#'H. B. McMahan, G. Holt, D. Sculley, et al. "Ad click
#'prediction: a view from the trenches". In: _The 19th ACM SIGKDD
#'International Conference on Knowledge Discovery and Data Mining,
#'KDD 2013, Chicago, IL, USA, August 11-14, 2013_. Ed. by I. S.
#'Dhillon, Y. Koren, R. Ghani, T. E. Senator, P. Bradley, R. Parekh,
#'J. He, R. L. Grossman and R. Uthurusamy. ACM, 2013, pp. 1222-1230.
#'DOI: 10.1145/2487575.2488200. <URL:
#'\url{http://doi.acm.org/10.1145/2487575.2488200}>.
#'
#'Kilian Q. Weinberger, Anirban Dasgupta, John Langford, 
#'Alexander J. Smola, and Josh Attenberg. ICML, volume 382 of ACM 
#'International Conference Proceeding Series, page 140. ACM, (2009)
#'
#'W. Zhang, S. Yuan, J. Wang, et al. "Real-Time Bidding
#'Benchmarking with iPinYou Dataset". In: _arXiv preprint
#'arXiv:1407.7073_ (2014).
#'@examples
#'# The following scripts show how to fit a logistic regression 
#'# after feature hashing
#'\dontrun{
#'data(ipinyou)
#'f <- ~ IP + Region + City + AdExchange + Domain +
#'  URL + AdSlotId + AdSlotWidth + AdSlotHeight +
#'  AdSlotVisibility + AdSlotFormat + CreativeID +
#'  Adid + split(UserTag, delim = ",")
#'# if the version of FeatureHashing is 0.8, please use the following command:
#'# m.train <- as(hashed.model.matrix(f, ipinyou.train, 2^16, transpose = FALSE), "dgCMatrix")
#'m.train <- hashed.model.matrix(f, ipinyou.train, 2^16)
#'m.test <- hashed.model.matrix(f, ipinyou.test, 2^16)
#'
#'# logistic regression with glmnet
#'
#'library(glmnet)
#'
#'cv.g.lr <- cv.glmnet(m.train, ipinyou.train$IsClick,
#'  family = "binomial")#, type.measure = "auc")
#'p.lr <- predict(cv.g.lr, m.test, s="lambda.min")
#'auc(ipinyou.test$IsClick, p.lr)
#'
#'## Per-Coordinate FTRL-Proximal with $L_1$ and $L_2$ Regularization for Logistic Regression
#'
#'# The following scripts use an implementation of the FTRL-Proximal for Logistic Regresion, 
#'# which is published in McMahan, Holt and Sculley et al. (2013), to predict the probability 
#'# (1-step prediction) and update the model simultaneously.
#'
#'
#'source(system.file("ftprl.R", package = "FeatureHashing"))
#'m.train <- hashed.model.matrix(f, ipinyou.train, 2^16, transpose = TRUE)
#'ftprl <- initialize.ftprl(0.1, 1, 0.1, 0.1, 2^16)
#'ftprl <- update.ftprl(ftprl, m.train, ipinyou.train$IsClick, predict = TRUE)
#'auc(ipinyou.train$IsClick, attr(ftprl, "predict"))
#'
#'# If we use the same algorithm to predict the click through rate of the 3rd season of iPinYou, 
#'# the overall AUC will be 0.77 which is comparable to the overall AUC of the 
#'# 3rd season 0.76 reported in Zhang, Yuan, Wang, et al. (2014).
#'}
#'
#'# The following scripts show the implementation of the FeatureHashing.
#'
#'# Below the original values will be project in a space of 2^6 dimensions
#'m <- hashed.model.matrix(~ ., CO2, 2^6, create.mapping = TRUE, 
#'  transpose = TRUE, is.dgCMatrix = FALSE)
#'  
#'# Print the matrix via dgCMatrix
#'as(m, "dgCMatrix")
#'
#'# Extraction of the dictionary: values with their hash
#'mapping <- hash.mapping(m)
#'
#'# To check the rate of collisions, we will extract the indices of the hash
#'# values through the modulo-division method, count how many duplicates 
#'# we have (in best case it should be zero) and perform a mean.
#'mean(duplicated(mapping))
#'
#'# The type of the result produced by the function `hashed.model.matrix` 
#'# is a CSCMatrix. It supports simple subsetting 
#'# and matrix-vector multiplication
#'rnorm(2^6) %*% m
#'
#'# Detail of the hashing
#'# To hash one specific value, we can use the `hashed.value` function
#'# Below we will apply this function to the feature names
#'vectHash <- hashed.value(names(mapping))
#'
#'# Now we will check that the result is the same than the one got with 
#'# the more generation `hashed.model.matrix` function.
#'# We will use the Modulo-division method (that's the [%% 2^6] below) 
#'# to find the address in hash table easily.
#'stopifnot(all(vectHash %% 2^6 + 1 == mapping))
#'
#'# The sign is corrected by `hash.sign`
#'hash.sign(names(mapping))
#'
#'## The interaction term is implemented as follow:
#'m2 <- hashed.model.matrix(~ .^2, CO2, 2^6, create.mapping = TRUE, 
#'  transpose = TRUE, is.dgCMatrix = FALSE)
#'# The ^ operator indicates crossing to the specified degree. 
#'# For example (a+b+c)^2 is identical to (a+b+c)*(a+b+c) 
#'# which in turn expands to a formula containing the main effects
#'# for a, b and c together with their second-order interactions. 
#'      
#'# Extract the mapping
#'mapping2 <- hash.mapping(m2)
#'
#'# Get the hash of combination of two items, PlantQn2 and uptake 
#'mapping2["PlantQn2:uptake"] 
#'
#'# Extract hash of each item
#'h1 <- hashed.value("PlantQn2")
#'h2 <- hashed.value("uptake")
#'
#'# Computation of hash of both items combined
#'h3 <- hashed.value(rawToChar(c(intToRaw(h1), intToRaw(h2)))) 
#'stopifnot(h3 %% 2^6 + 1 == mapping2["PlantQn2:uptake"])
#'
#'# The concatenated feature, i.e. the array<string> type in hive
#'data(test.tag)
#'df <- data.frame(a = test.tag, b = rnorm(length(test.tag)))
#'m <- hashed.model.matrix(~ split(a, delim = ",", type = "existence"):b, df, 2^6,
#'  create.mapping = TRUE)
#'# The column `a` is splitted by "," and have an interaction with "b":
#'mapping <- hash.mapping(m)
#'names(mapping)
#'
#'@export
#'@importFrom methods new as checkAtAssignment as
#'@importFrom stats as.formula terms.formula
#'@importFrom utils getParseData head
#'@importClassesFrom Matrix dgCMatrix
#'@aliases hashed.value hash.sign hashed.interaction.value
hashed.model.matrix <- function(formula, data, hash.size = 2^18, transpose = FALSE, 
                                create.mapping = FALSE, is.dgCMatrix = TRUE, signed.hash = FALSE,
                                progress = FALSE) {
  stopifnot(hash.size >= 0)
  stopifnot(is.data.frame(data))
  stopifnot(class(formula) %in% c("formula", "character"))
  
  if(class(formula) == "character") formula %<>% paste(collapse = " + ") %>% paste("~", .) %>% as.formula
  
  tf.idf.string <- "type = \"tf-idf\""
  
  tf.idf <- as.character(formula) %>% grep(tf.idf.string, .) %>% sum > 1
  
  if(tf.idf){
    if(signed.hash) stop("If you use tf-idf, parameter signed.hash should be set to FALSE.")
    formula <- as.character(formula) %>% gsub(pattern = tf.idf.string, replacement = "type = \"count\"", x = .) %>% paste0(collapse = " ") %>% as.formula
  }
  
  tf <- terms.formula(formula, data = data, specials = "split")
  retval <- new(.CSCMatrix)
  .hashed.model.matrix.dataframe(tf, data, hash.size, transpose, retval, create.mapping, signed.hash, progress)
  class(retval) <- .CSCMatrix
  retval@Dimnames[[2]] <- paste(seq_len(retval@Dim[2]))
  if (is.dgCMatrix) {
    retval2 <- as(retval, "dgCMatrix") 
    for(name in setdiff(names(attributes(retval)), names(attributes(retval2)))) {
      attr(retval2, name) <- attr(retval, name)
    }
    if (tf.idf) tf.idf.transfo(retval2) else retval2
  } else if (tf.idf) tf.idf.transfo(retval) else retval
}

# This is the function called from C to parse the \code{split} function.
parse_split <- function(text) {
  origin.keep.source <- options()$keep.source
  tryCatch({
    options(keep.source = TRUE)
    p <- parse(text = text)
    tmp <- getParseData(p)
    reference_name <- tmp$text[which(tmp$token == "SYMBOL")]
    if ("delim" %in% tmp$text) {
      delim <- tmp$text[which(tmp$text == "delim")[1] + 2]
      delim <- gsub(pattern = '"', replacement = '', delim)
    } else {
      # the default value of delim
      delim <- ","
    }
    if ("type" %in% tmp$text) {
      type <- tmp$text[which(tmp$text == "type")[1] + 2]
      type <- gsub(pattern = '"', replacement = '', type)
    } else {
      # the default value of type
      type <- "existence"
    }
    list(reference_name = reference_name, delim = delim, type = type)
  }, finally = {options(keep.source = origin.keep.source)})
}

tf.idf.transfo <- function(hash.matrix){
  idf.train <- log(nrow(hash.matrix)/colSums(hash.matrix)) %>% Diagonal(x = .)
  hash.matrix %*% idf.train
}

# Avoid error messages during CRAN check.
# The reason is that these variables are never declared
# They are mainly column names inferred by Data.table...
globalVariables(c("."))