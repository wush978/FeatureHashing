#include <cstring>
#include <Rcpp.h>
#include "tag.h"
using namespace Rcpp;

std::vector<std::string> split(const std::string& src, const std::string& delim) {
  const char* start = src.c_str();
  const char* end = std::strstr(start, delim.c_str());
  std::vector<std::string> retval;
  while(end != NULL) {
    retval.push_back(std::string(start, end));
    start = end + delim.size();
    end = std::strstr(start, delim.c_str());
  }
  retval.push_back(std::string(start));
  return retval;
}

//[[Rcpp::export]]
SEXP tag_existence(CharacterVector src, const std::string& delim) {
  std::map<std::string, LogicalVector> retval_buffer;
  for(auto i = 0;i < src.size();i++) {
    std::vector<std::string> tokens(split(CHAR(STRING_ELT(src, i)), delim));
    for(auto j = tokens.begin();j != tokens.end();j++) {
      if (j->size() == 0) continue;
      auto k = retval_buffer.find(*j);
      if (k == retval_buffer.end()) {
        auto tmp = retval_buffer.insert(std::pair<std::string, LogicalVector>(*j, LogicalVector(src.size(), 0)));
        k = tmp.first;
      }
      k->second[i] = 1;
    }
  }
  List retval(retval_buffer.size());
  CharacterVector retval_name(retval_buffer.size());
  size_t j = 0;
  for(auto i = retval_buffer.begin();i != retval_buffer.end();i++) {
    retval[j] = i->second;
    retval_name[j++] = i->first;
  }
  retval.attr("names") = retval_name;
  return retval;
}

//[[Rcpp::export]]
SEXP tag_count(CharacterVector src, const std::string& delim) {
  std::map<std::string, IntegerVector> retval_buffer;
  for(auto i = 0;i < src.size();i++) {
    std::vector<std::string> tokens(split(CHAR(STRING_ELT(src, i)), delim));
    for(auto j = tokens.begin();j != tokens.end();j++) {
      if (j->size() == 0) continue;
      auto k = retval_buffer.find(*j);
      if (k == retval_buffer.end()) {
        auto tmp = retval_buffer.insert(std::pair<std::string, IntegerVector>(*j, IntegerVector(src.size(), 0)));
        k = tmp.first;
      }
      k->second[i] += 1;
    }
  }
  List retval(retval_buffer.size());
  CharacterVector retval_name(retval_buffer.size());
  size_t j = 0;
  for(auto i = retval_buffer.begin();i != retval_buffer.end();i++) {
    retval[j] = i->second;
    retval_name[j++] = i->first;
  }
  retval.attr("names") = retval_name;
  return retval;  
}
