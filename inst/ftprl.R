initialize.ftprl <- function(alpha, beta, lambda1, lambda2, nfeature) {
  list(
    z = numeric(nfeature),
    n = numeric(nfeature),
    alpha = alpha,
    beta = beta,
    lambda1 = lambda1, 
    lambda2 = lambda2
    )
}

predict.ftprl <- function(ftprl, m) {
  w.ftprl <- function(ftprl, i) {
    retval <- numeric(length(i))
    index <- which(abs(ftprl$z[i]) > ftprl$lambda1)
    i <- i[index]
    retval[index] <- 
      - (ftprl$z[i] - sign(ftprl$z[i]) * ftprl$lambda1) / (ftprl$lambda2 + (ftprl$beta + sqrt(ftprl$n[i])) / ftprl$alpha)
    retval
  }
  sigma <- function(x) 1 / (1 + exp(-x))
  .predict.ftprl <- function(ftprl, j, x) {
    w <- w.ftprl(ftprl, j)
    sigma(sum(w * x))
  }
  p <- numeric(ncol(m))
  if (interactive()) pb <- txtProgressBar(max = ncol(m), style = 3)
  for(col in seq_len(ncol(m))) {
    index <- if (m@p[col] == m@p[col + 1]) integer(0) else seq.int(m@p[col], m@p[col + 1] - 1, by = 1L)
    p[col] <- .predict.ftprl(ftprl, i <- m@i[index + 1] + 1, x <- m@x[index + 1])
    if (interactive()) setTxtProgressBar(pb, col)
  }
  if (interactive()) close(pb)
  p
}

update.ftprl <- function(ftprl, m, y, predict = TRUE) {
  w.ftprl <- function(ftprl, i) {
    retval <- numeric(length(i))
    index <- which(abs(ftprl$z[i]) > ftprl$lambda1)
    i <- i[index]
    retval[index] <- 
      - (ftprl$z[i] - sign(ftprl$z[i]) * ftprl$lambda1) / (ftprl$lambda2 + (ftprl$beta + sqrt(ftprl$n[i])) / ftprl$alpha)
    retval
  }
  sigma <- function(x) 1 / (1 + exp(-x))
  .predict.ftprl <- function(ftprl, j, x) {
    w <- w.ftprl(ftprl, j)
    sigma(sum(w * x))
  }
  p <- numeric(ncol(m))
  if (interactive()) pb <- txtProgressBar(max = ncol(m), style = 3)
  for(col in seq_len(ncol(m))) {
    index <- if (m@p[col] == m@p[col + 1]) integer(0) else seq.int(m@p[col], m@p[col + 1] - 1, by = 1L)
    p[col] <- .predict.ftprl(ftprl, i <- m@i[index + 1] + 1, x <- m@x[index + 1])
    g <- (p[col] - y[col]) * x
    s <- (sqrt(ftprl$n[i] + g * g) - sqrt(ftprl$n[i])) / ftprl$alpha
    ftprl$z[i] <- ftprl$z[i] + g - s * w.ftprl(ftprl, i)
    ftprl$n[i] <- ftprl$n[i] + g * g
    if (interactive()) setTxtProgressBar(pb, col)
  }
  if (interactive()) close(pb)
  attr(ftprl, "predict") <- p
  ftprl
}
