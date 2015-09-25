if (require(RUnit) & Sys.getenv("TEST_JIEBAR") == "TRUE") {
  library(FeatureHashing)
  df <- data.frame(title = c(
    "貶值取代降息？ 台幣貶破33元",
    "優生 培寶4款毒奶瓶下架",
    " 秋節上國道 閃11塞車點",
    "習近平訪美前…//中國戰機公海危險攔截美機",
    "352億公開收購 日月光成矽品最大股東",
    "驚 AT-3又出事 南投深山失聯 2飛官生死未卜",
    "誰說該廢死的？怕死鄭捷首度道歉",
    "歐習會前夕// 美國安顧問：反對片面改變台海現狀"
  ))
  init_jiebaR_callback()
  m <- hashed.model.matrix(~ jiebaR(title), df, create.mapping = TRUE)
  title_tokens <- names(hash.mapping(m))
  checkEquals(title_tokens, c("title4", "title股東", "title國道", "title中國", "title現狀", 
"title…", "title閃", "title習近平", "title日", "title11", 
"title驚", "title公開", "title億", "title又", "title：", 
"title該", "title塞車", "title訪美", "title？", "title會", 
"title公海", "title深山", "title片面", "title奶瓶", "title說", 
"title成矽品", "title危險", "title台海", "title最大", 
"title美國", "title貶值", "title上", "title下架", "title秋節", 
"titleAT", "title352", "title生死未卜", "title收購", "title月光", 
"title怕死", "title貶破", "title飛官", "title出事", "title取代", 
"title道歉", "title歐習", "title33", "title ", "title款毒", 
"title優生", "title顧問", "title前", "title前夕", "title廢死的", 
"title反對", "title改變", "title點", "title培寶", "title台幣", 
"title降息", "title美機", "title安", "title-", "title南投", 
"title首度", "title戰機", "title鄭捷", "title/", "title元", 
"title誰", "title攔截", "title2", "title失聯", "title3"))
  m <- hashed.model.matrix(~ jiebaR(title, type = "hmm"), df, create.mapping = TRUE)
  title_tokens <- names(hash.mapping(m))
  checkEquals(title_tokens, c("title4", "title改", "title鄭", "title股東", "title死", 
"title…", "title海", "title上國道", "title閃", "title日", 
"title11", "title現", "title首", "title驚", "title片", "title光成", 
"title又", "title：", "title該", "title機公海", "title反", 
"title習近", "title矽品", "title怕", "title生死", "title？", 
"title捷", "title會", "title對", "title深山", "title奶瓶", 
"title說", "title月", "title危險", "title最大", "title貶值", 
"title下架", "title台", "title秋節", "titleAT", "title美前", 
"title面", "title352", "title收購", "title狀", "title貶破", 
"title飛官", "title歉", "title出事", "title取代", "title平訪", 
"title歐習", "title億公開", "title33", "title未卜", "title中國戰", 
"title ", "title款毒", "title優生", "title前夕", "title度", 
"title美國安顧問", "title廢死的", "title變", "title塞車點", 
"title培寶", "title台幣", "title降息", "title美機", "title-", 
"title南投", "title道", "title/", "title元", "title誰", 
"title攔截", "title2", "title失聯", "title3"))
}