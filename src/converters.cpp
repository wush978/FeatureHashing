#include "converters.h"

const std::vector<uint32_t>& CallbackConverter::get_feature(size_t i) {
  SEXP pstr = STRING_ELT(psrc, i);
  if (pstr == NA_STRING) {
    feature_buffer.clear();
  } else {
    const char* str = CHAR(pstr);
    cache = f->operator()(str);
    feature_buffer.resize(cache.size());
    std::transform(cache.begin(), cache.end(), feature_buffer.begin(), 
                   [this](const std::string& s) {
                     return this->get_hashed_feature(this->h_main, s.c_str());
                   });
    if (is_final) {
      std::transform(feature_buffer.begin(), feature_buffer.end(), 
        feature_buffer.begin(), [this](uint32_t feature) {
          return feature % this->hash_size;
        });
      if (f->decollision) {
        std::set<uint32_t> tmp(feature_buffer.begin(), feature_buffer.end());
        feature_buffer.clear();
        feature_buffer.assign(tmp.begin(), tmp.end());
      }
    }
  }
  return feature_buffer;
}

const std::vector<double>& CallbackConverter::get_value(size_t i) {
  SEXP pstr = STRING_ELT(psrc, i);
  if (pstr == NA_STRING) {
    value_buffer.clear();
  } else {
    const char* str = CHAR(pstr);
    value_buffer.resize(cache.size());
    std::transform(cache.begin(), cache.end(), value_buffer.begin(), 
                   [this](const std::string& s) {
                     return get_sign(get_hashed_feature(this->h_binary, s.c_str()));
                   });
#ifdef NOISY_DEBUG
    for(int j = 0;j < cache.size();j++) {
      Rprintf("signed hash: %s ... got %zu\n", cache[j].c_str(), value_buffer[j]);
    }
#endif
    if (is_final & f->decollision) {
      if (value_buffer.size() < feature_buffer.size()) throw std::logic_error("The length of value_buffer and feature_buffer go wrong!");
      value_buffer.resize(feature_buffer.size());
    }
  }
  return value_buffer;
}

