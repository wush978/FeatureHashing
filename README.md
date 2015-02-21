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

### When should we use Feature Hashing?

Feature hashing is useful when the user do not easy to know the dimension of the feature vector. For example, the bag-of-word representation in document classification problem requires scanning entire dataset to know how many words we have, i.e. the dimension of the feature vector.

In general, feature hashing is useful in the following environment:

- Streaming Environment
- Distirbuted Environment

Because it is expensive or impossible to scan entire dataset.

## Getting Started

The following scripts show how to use the `FeatureHashing` to construct `Matrix::dgCMatrix` and train a model in other packages which supports `Matrix::dgCMatrix` as input.

### Logistic Regression with [`glmnet`](http://cran.r-project.org/web/packages/glmnet/index.html)

```r
# The following script assumes that the data.frame
# of the training dataset and testing dataset are 
# assigned to variable `imp.train` and `imp.test`
# respectively

library(FeatureHashing)
 
f <- ~ ip + region + city + adexchange + domain +
  URL + AdSlotId + AdSlotWidth + AdSlotHeight +
  AdSlotVisibility + AdSlotFormat + CreativeID +
  weekday + hour + tag(usertag, split = ",")
m.train <- hashed.model.matrix(f, imp.train, 2^20)
m.test <- hashed.model.matrix(f, imp.test, 2^20)

# logistic regression with glmnet

library(glmnet)

cv.g.lr <- cv.glmnet(m.train, imp.train$is_click,
  family = "binomial", type.measure = "auc")
plot(cv.g.lr)
cv.g.lr$lambda.min
# coef(cv.g.lm, s = "lambda.min")
p.lr <- predict(cv.g.lr, m.test, s="lambda.min")
auc(imp.test$is_click, p.lr)
```

### Gradient Boosted Decision Tree with [`xgboost`](http://cran.r-project.org/web/packages/xgboost/index.html)

Following the script above, 

```r
# GBDT with xgboost

library(xgboost)

cv.g.gdbt <- xgboost(m.train, imp.train$is_click, max.depth=7, eta=0.1,
  nround = 100, objective = "binary:logistic")
p.lm <- predict(cv.g.gdbt, m.test)
glmnet::auc(imp.test$is_click, p.lm)
```

## Supported Data Structure

- character and factor
- numeric and integer
- array, i.e. concatenated strings such as `c("a,b", "a,b,c", "a,c", "")`
