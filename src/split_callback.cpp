#include "callback.h"
#include "split.h"
#include <Rcpp.h>

struct SplitCallbackFunctor : public CallbackFunctor {
  
  enum SplitType {
    Count,
    Existence
  };

  std::string delim;
  SplitType type;
  
  SplitCallbackFunctor(SEXP input, const std::string& _delim, const std::string& _type) 
    : delim(_delim), CallbackFunctor(input)
    { 
      set_type(_type);
      if (type == SplitType::Existence) decollision = true;
    }
  
  virtual ~SplitCallbackFunctor() { }
  
  void set_type(std::string _type) {
    if (_type.compare("count") == 0) {
      type = SplitType::Count;
    } else if (_type.compare("existence") == 0) {
      type = SplitType::Existence;
    } else throw std::invalid_argument("Not supported type");
  }
  
  std::string get_type() {
    switch (type) {
    case SplitType::Count:
      return "count";
    case SplitType::Existence:
      return "existence";
    }
    throw std::logic_error("Invalid SplitType");
  }
  
  virtual const std::vector<std::string> operator()(const char* input) const {
    switch (type) {
    case SplitType::Count: {
      auto tmp(split(input, delim));
      tmp.erase(std::remove(tmp.begin(), tmp.end(), ""), tmp.end());
      return tmp;
    }
    case SplitType::Existence: {
      std::vector<std::string> tmp(split(input, delim));
      std::set<std::string> tmp2(tmp.begin(), tmp.end());
      tmp2.erase("");
      tmp.assign(tmp2.begin(), tmp2.end());
      return tmp;
    }
    }
    throw std::logic_error("Invalid SplitType");
  }
  
};

using namespace Rcpp;

RCPP_MODULE(split_callback) {

  class_<CallbackFunctor>("callback")
  ;
    
  class_<SplitCallbackFunctor>("split_callback")
  .derives<CallbackFunctor>("callback")
  .constructor<SEXP, std::string, std::string>()
  .field("delim", &SplitCallbackFunctor::delim)
  .property("type", &SplitCallbackFunctor::get_type, &SplitCallbackFunctor::set_type)
  ;
  
}