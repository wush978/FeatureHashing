// [[Rcpp::depends(jiebaR)]]
// [[Rcpp::depends(FeatureHashing)]]

#include "jiebaRAPI.h"
#include <callback.h>
#include <Rcpp.h>

using namespace Rcpp;

struct jiebaRCallbackFunctor : public CallbackFunctor {
  
  enum Type {
    MIX,
    MP,
    HMM,
    QUERY,
    KEY
  };

  Type type;
  Environment cutter;
  SEXP cutter_pointer;

  typedef SEXP (*Cut)(SEXP, SEXP);
  
  Cut cut;
  
  void set_type(std::string _type) {
    if (_type.compare("mix") == 0) {
      type = MIX;
    } else if (_type.compare("mp") == 0) {
      type = MP;
    } else if (_type.compare("hmm") == 0) {
      type = HMM;
    } else if (_type.compare("query") == 0) {
      type = QUERY;
    } else if (_type.compare("key") == 0) {
      type = KEY;
    } else {
      throw std::invalid_argument("Unknown type");
    }
  }
  
  std::string get_type() {
    switch (type) {
    case MIX:
      return "mix";
    case MP:
      return "mp";
    case HMM:
      return "hmm";
    case QUERY:
      return "query";
    case KEY:
      return "key";
    }
  }
  
  void set_cut() {
    std::string fname("jiebaR_");
    fname.append(get_type());
    fname.append("_cut");
    cut = reinterpret_cast<Cut>(::R_GetCCallable("jiebaR", fname.c_str()));
  }
  
  explicit jiebaRCallbackFunctor(
    SEXP _src,
    std::string _type,
    SEXP _cutter
  ) 
    : type(MIX), 
      cutter(_cutter),
      cutter_pointer(NULL),
      cut(NULL),
      CallbackFunctor(_src)
      {
        set_type(_type);
        set_cut();
        cutter_pointer = wrap(cutter["worker"]);
      }
  
  virtual ~jiebaRCallbackFunctor() { }
  
  virtual const std::vector<std::string> operator()(const char* input) const {
    return as<std::vector<std::string> >((*cut)(wrap(input), cutter_pointer));
  }
  
};

RCPP_MODULE(jiebaR_callback) {

  class_<CallbackFunctor>("callback")
  ;
    
  class_<jiebaRCallbackFunctor>("jiebaR_callback")
  .derives<CallbackFunctor>("callback")
  .constructor<SEXP, std::string, SEXP>()
  .property("type", &jiebaRCallbackFunctor::get_type, &jiebaRCallbackFunctor::set_type)
  .field("cutter", &jiebaRCallbackFunctor::cutter)
  ;

}

/***R
generate_jiebaR_callback <- function(input, type = "mix", ...) {
  worker <- jiebaR::worker(type = type, ...)
  callback <- new(jiebaR_callback, input, type, worker)
  callback
}

FeatureHashing::register_callback("jiebaR", generate_jiebaR_callback)
*/
