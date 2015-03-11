FeatureHashing
==============

Implement feature hashing with R

## Introduction

[Feature hashing](http://en.wikipedia.org/wiki/Feature_hashing), also called as the hashing trick, is a method to
transform features to vector. Without looking the indices up in an
associative array, it applies a hash function to the features and uses their
hash values as indices directly.
  
The package FeatureHashing implements the method in Weinberger et. al. (2009) to transform
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

### Logistic Regression with [`glmnet`](http://cran.r-project.org/web/packages/glmnet/index.html)


```r
# The following script assumes that the data.frame
# of the training dataset and testing dataset are 
# assigned to variable `imp.train` and `imp.test`
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
# m.train <- as(hashed.model.matrix(f, imp.train, 2^20, transpose = FALSE), "dgCMatrix")
m.train <- hashed.model.matrix(f, imp.train, 2^20)
m.test <- hashed.model.matrix(f, imp.test, 2^20)

# logistic regression with glmnet

library(glmnet)
```

```
## Loading required package: Matrix
## Loaded glmnet 1.9-8
```

```r
cv.g.lr <- cv.glmnet(m.train, imp.train$IsClick,
  family = "binomial")#, type.measure = "auc")
p.lr <- predict(cv.g.lr, m.test, s="lambda.min")
auc(imp.test$IsClick, p.lr)
```

```
## [1] 0.5199
```

### Gradient Boosted Decision Tree with [`xgboost`](http://cran.r-project.org/web/packages/xgboost/index.html)

Following the script above, 


```r
# GBDT with xgboost

library(xgboost)

cv.g.gdbt <- xgboost(m.train, imp.train$IsClick, max.depth=7, eta=0.1,
  nround = 100, objective = "binary:logistic")
```

```
## [0]	train-error:0.034278
## [1]	train-error:0.034278
## [2]	train-error:0.034278
## [3]	train-error:0.034278
## [4]	train-error:0.034278
## [5]	train-error:0.035125
## [6]	train-error:0.035125
## [7]	train-error:0.035125
## [8]	train-error:0.035125
## [9]	train-error:0.035125
## [10]	train-error:0.035125
## [11]	train-error:0.035125
## [12]	train-error:0.035125
## [13]	train-error:0.035125
## [14]	train-error:0.035125
## [15]	train-error:0.035125
## [16]	train-error:0.035125
## [17]	train-error:0.035125
## [18]	train-error:0.035125
## [19]	train-error:0.035125
## [20]	train-error:0.034702
## [21]	train-error:0.034278
## [22]	train-error:0.034278
## [23]	train-error:0.034278
## [24]	train-error:0.034278
## [25]	train-error:0.034278
## [26]	train-error:0.034278
## [27]	train-error:0.034278
## [28]	train-error:0.034278
## [29]	train-error:0.034278
## [30]	train-error:0.034278
## [31]	train-error:0.034278
## [32]	train-error:0.034278
## [33]	train-error:0.034278
## [34]	train-error:0.034278
## [35]	train-error:0.034278
## [36]	train-error:0.033855
## [37]	train-error:0.034278
## [38]	train-error:0.033855
## [39]	train-error:0.033432
## [40]	train-error:0.033855
## [41]	train-error:0.033855
## [42]	train-error:0.033855
## [43]	train-error:0.033855
## [44]	train-error:0.033855
## [45]	train-error:0.033432
## [46]	train-error:0.033432
## [47]	train-error:0.033432
## [48]	train-error:0.033009
## [49]	train-error:0.032586
## [50]	train-error:0.032586
## [51]	train-error:0.032163
## [52]	train-error:0.032586
## [53]	train-error:0.032163
## [54]	train-error:0.032163
## [55]	train-error:0.032163
## [56]	train-error:0.032163
## [57]	train-error:0.032163
## [58]	train-error:0.031739
## [59]	train-error:0.032163
## [60]	train-error:0.032163
## [61]	train-error:0.032163
## [62]	train-error:0.031739
## [63]	train-error:0.031739
## [64]	train-error:0.031739
## [65]	train-error:0.030893
## [66]	train-error:0.030893
## [67]	train-error:0.030893
## [68]	train-error:0.030893
## [69]	train-error:0.030893
## [70]	train-error:0.030893
## [71]	train-error:0.030893
## [72]	train-error:0.030470
## [73]	train-error:0.030470
## [74]	train-error:0.030470
## [75]	train-error:0.030470
## [76]	train-error:0.030470
## [77]	train-error:0.030470
## [78]	train-error:0.030470
## [79]	train-error:0.030470
## [80]	train-error:0.030047
## [81]	train-error:0.029623
## [82]	train-error:0.029200
## [83]	train-error:0.029200
## [84]	train-error:0.028777
## [85]	train-error:0.028777
## [86]	train-error:0.028777
## [87]	train-error:0.028777
## [88]	train-error:0.028777
## [89]	train-error:0.028777
## [90]	train-error:0.028777
## [91]	train-error:0.027931
## [92]	train-error:0.028354
## [93]	train-error:0.027931
## [94]	train-error:0.027931
## [95]	train-error:0.027931
## [96]	train-error:0.027931
## [97]	train-error:0.027931
## [98]	train-error:0.027931
## [99]	train-error:0.027931
```

```r
p.lm <- predict(cv.g.gdbt, m.test)
glmnet::auc(imp.test$IsClick, p.lm)
```

```
## [1] 0.6497
```

## Supported Data Structure

- character and factor
- numeric and integer
- array, i.e. concatenated strings such as `c("a,b", "a,b,c", "a,c", "")`
