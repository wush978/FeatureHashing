FeatureHashing
==============

Implement feature hashing with R



## Introduction

[Feature hashing](http://en.wikipedia.org/wiki/Feature_hashing), also called as the hashing trick, is a method to
transform features to vector. Without looking the indices up in an
associative array, it applies a hash function to the features and uses their
hash values as indices directly.
  
The package FeatureHashing implements the method in (Weinberger, Dasgupta, Langford, Smola, and Attenberg, 2009) to transform
a `data.frame` to sparse matrix. The package provides a formula interface similar to model.matrix 
in R and Matrix::sparse.model.matrix in the package Matrix. Splitting of concatenated data, 
check the help of `test.tag` for explanation of concatenated data, during the construction of the model matrix.

## Installation

To install the stable version from Cran, run this command:
```r
install.packages("FeatureHashing")
```

For up-to-date version, please install from github. Windows user will need to install [RTools](http://cran.r-project.org/bin/windows/Rtools/) first.

```r
devtools::install_github('wush978/FeatureHashing')
```

### When should we use Feature Hashing?

Feature hashing is useful when the user does not easy to know the dimension of the feature vector. 
For example, the bag-of-word representation in document classification problem requires scanning entire dataset to know how many words we have, i.e. the dimension of the feature vector.

In general, feature hashing is useful in the following environment:

- Streaming Environment
- Distirbuted Environment

Because it is expensive or impossible to know the real dimension of the feature vector.

## Getting Started

The following scripts show how to use the `FeatureHashing` to construct `Matrix::dgCMatrix` and train a model in other packages which supports `Matrix::dgCMatrix` as input.

The dataset is a sample from iPinYou dataset which is described in (Zhang, Yuan, Wang, and Shen, 2014).

### Logistic Regression with [`glmnet`](http://cran.r-project.org/web/packages/glmnet/index.html)


```r
# The following script assumes that the data.frame
# of the training dataset and testing dataset are 
# assigned to variable `ipinyou.train` and `ipinyou.test`
# respectively

library(FeatureHashing)
```

```
## Loading required package: methods
```

```r
# Checking version.
stopifnot(packageVersion("FeatureHashing") >= package_version("0.9"))

data(ipinyou)
f <- ~ IP + Region + City + AdExchange + Domain +
  URL + AdSlotId + AdSlotWidth + AdSlotHeight +
  AdSlotVisibility + AdSlotFormat + CreativeID +
  Adid + split(UserTag, delim = ",")
# if the version of FeatureHashing is 0.8, please use the following command:
# m.train <- as(hashed.model.matrix(f, ipinyou.train, 2^16, transpose = FALSE), "dgCMatrix")
m.train <- hashed.model.matrix(f, ipinyou.train, 2^16)
m.test <- hashed.model.matrix(f, ipinyou.test, 2^16)

# logistic regression with glmnet

library(glmnet)
```

```
## Loading required package: Matrix
## Loaded glmnet 1.9-8
```

```r
cv.g.lr <- cv.glmnet(m.train, ipinyou.train$IsClick,
  family = "binomial")#, type.measure = "auc")
p.lr <- predict(cv.g.lr, m.test, s="lambda.min")
auc(ipinyou.test$IsClick, p.lr)
```

```
## [1] 0.5077
```

### Gradient Boosted Decision Tree with [`xgboost`](http://cran.r-project.org/web/packages/xgboost/index.html)

Following the script above, 


```r
# GBDT with xgboost

library(xgboost)

cv.g.gdbt <- xgboost(m.train, ipinyou.train$IsClick, max.depth=7, eta=0.1,
  nround = 100, objective = "binary:logistic", verbose = ifelse(interactive(), 1, 0))
p.lm <- predict(cv.g.gdbt, m.test)
glmnet::auc(ipinyou.test$IsClick, p.lm)
```

```
## [1] 0.6555
```


### Per-Coordinate FTRL-Proximal with $L_1$ and $L_2$ Regularization for Logistic Regression

The following scripts use an implementation of the FTRL-Proximal for Logistic Regresion, which is published in (McMahan, Holt, Sculley, Young, Ebner, Grady, Nie, Phillips, Davydov, Golovin, Chikkerur, Liu, Wattenberg, Hrafnkelsson, Boulos, and Kubica, 2013), to predict the probability (1-step prediction) and update the model simultaneously.



```r
source(system.file("ftprl.R", package = "FeatureHashing"))

m.train <- hashed.model.matrix(f, ipinyou.train, 2^16, transpose = TRUE)
ftprl <- initialize.ftprl(0.1, 1, 0.1, 0.1, 2^16)
ftprl <- update.ftprl(ftprl, m.train, ipinyou.train$IsClick, predict = TRUE)
auc(ipinyou.train$IsClick, attr(ftprl, "predict"))
```

```
## [1] 0.5986
```

If we use the same algorithm to predict the click through rate of the 3rd season of iPinYou, the overall AUC will be 0.77 which is comparable to the overall AUC of the 3rd season 0.76 reported in (Zhang, Yuan, Wang, et al., 2014).

## Supported Data Structure

- character and factor
- numeric and integer
- array, i.e. concatenated strings such as `c("a,b", "a,b,c", "a,c", "")`

## Reference

[1] H. B. McMahan, G. Holt, D. Sculley, et al. "Ad click
prediction: a view from the trenches". In: _The 19th ACM SIGKDD
International Conference on Knowledge Discovery and Data Mining,
KDD 2013, Chicago, IL, USA, August 11-14, 2013_. Ed. by I. S.
Dhillon, Y. Koren, R. Ghani, T. E. Senator, P. Bradley, R. Parekh,
J. He, R. L. Grossman and R. Uthurusamy. ACM, 2013, pp. 1222-1230.
DOI: 10.1145/2487575.2488200. <URL:
http://doi.acm.org/10.1145/2487575.2488200>.

[2] K. Q. Weinberger, A. Dasgupta, J. Langford, et al. "Feature
hashing for large scale multitask learning". In: _Proceedings of
the 26th Annual International Conference on Machine Learning, ICML
2009, Montreal, Quebec, Canada, June 14-18, 2009_. Ed. by A. P.
Danyluk, L. Bottou and M. L. Littman. 2009, pp. 1113-1120. DOI:
10.1145/1553374.1553516. <URL:
http://doi.acm.org/10.1145/1553374.1553516>.

[3] W. Zhang, S. Yuan, J. Wang, et al. "Real-Time Bidding
Benchmarking with iPinYou Dataset". In: _arXiv preprint
arXiv:1407.7073_ (2014).
