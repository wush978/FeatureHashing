library(FeatureHashing)
data <- data.frame(a = c("1,2,3", "2,3,3", "1,3", "3"), type = c("a", "b", "a", "a"), stringsAsFactors = FALSE)
object <- ~ tag(a, split = ",") + type
r <- interpret.tag(~ tag(a)*type, data)
stopifnot(isTRUE(all.equal(r, 
structure(list(object = ~a_count__1 + a_count__2 + a_count__3 + 
    type + a_count__1:type + a_count__2:type + a_count__3:type, 
    data = structure(list(a = c("1,2,3", "2,3,3", "1,3", "3"), 
        type = c("a", "b", "a", "a"), a_count__1 = c(1L, 0L, 
        1L, 0L), a_count__2 = c(1L, 1L, 0L, 0L), a_count__3 = c(1L, 
        2L, 1L, 1L)), .Names = c("a", "type", "a_count__1", "a_count__2", 
    "a_count__3"), row.names = c(NA, -4L), class = "data.frame")), .Names = c("object", 
"data"))
)))

data$a <- gsub(",", ";", data$a)
r <- interpret.tag(~ tag(a, split = ";")*type, data)
stopifnot(isTRUE(all.equal(r, 
structure(list(object = ~a_count__1 + a_count__2 + a_count__3 + 
    type + a_count__1:type + a_count__2:type + a_count__3:type, 
    data = structure(list(a = c("1;2;3", "2;3;3", "1;3", "3"), 
        type = c("a", "b", "a", "a"), a_count__1 = c(1L, 0L, 
        1L, 0L), a_count__2 = c(1L, 1L, 0L, 0L), a_count__3 = c(1L, 
        2L, 1L, 1L)), .Names = c("a", "type", "a_count__1", "a_count__2", 
    "a_count__3"), row.names = c(NA, -4L), class = "data.frame")), .Names = c("object", 
"data"))
)))

r <- interpret.tag(~ tag(a, split = ";", type = "existence") * tag(a, split = ";", type = "count"), data)
stopifnot(isTRUE(all.equal(r, 
structure(list(object = ~a_existence__1 + a_existence__2 + a_existence__3 + 
    a_count__1 + a_count__2 + a_count__3 + a_existence__1:a_count__1 + 
    a_existence__2:a_count__1 + a_existence__3:a_count__1 + a_existence__1:a_count__2 + 
    a_existence__2:a_count__2 + a_existence__3:a_count__2 + a_existence__1:a_count__3 + 
    a_existence__2:a_count__3 + a_existence__3:a_count__3, data = structure(list(
    a = c("1;2;3", "2;3;3", "1;3", "3"), type = c("a", "b", "a", 
    "a"), a_existence__1 = c(TRUE, FALSE, TRUE, FALSE), a_existence__2 = c(TRUE, 
    TRUE, FALSE, FALSE), a_existence__3 = c(TRUE, TRUE, TRUE, 
    TRUE), a_count__1 = c(1L, 0L, 1L, 0L), a_count__2 = c(1L, 
    1L, 0L, 0L), a_count__3 = c(1L, 2L, 1L, 1L)), .Names = c("a", 
"type", "a_existence__1", "a_existence__2", "a_existence__3", 
"a_count__1", "a_count__2", "a_count__3"), row.names = c(NA, 
-4L), class = "data.frame")), .Names = c("object", "data"))
)))
