#include <cstring>
#include <deque>
#include <boost/algorithm/string.hpp>
#include "hash_function.h"
#include "pmurhashAPI.h"
#include "bswap_32.h"
#include <Rcpp.h>
#include "hashed_model_matrix.h"

const uint32_t 
  MURMURHASH3_H_SEED = 3120602769LL,
  MURMURHASH3_XI_SEED = 79193439LL;

uint32_t NullHashFunction::operator()(const char* buf, int size, bool is_interaction) {
  return 1;
}

uint32_t MurmurHash3HashFunction::operator()(const char* buf, int size, bool is_interaction) {
  return ::PMurHash32(seed, buf, size);
}

uint32_t MurmurHash3LogHashFunction::operator()(const char* buf, int size, bool is_interaction) {
  uint32_t retval = PMurHash32(seed, buf, size);
  if (is_interaction) {
    const uint32_t* src = reinterpret_cast<const uint32_t*>(buf);
    #ifdef BOOST_BIG_ENDIAN
    if (inverse_mapping.find(bswap_32(src[0])) == inverse_mapping.end()) throw std::logic_error("interaction is hashed before main effect!");
    if (inverse_mapping.find(bswap_32(src[1])) == inverse_mapping.end()) throw std::logic_error("interaction is hashed before main effect!");
    std::string key(inverse_mapping[bswap_32(src[0])]);
    key.append(":");
    key.append(inverse_mapping[bswap_32(src[1])]);
    #else
    if (inverse_mapping.find(src[0]) == inverse_mapping.end()) throw std::logic_error("interaction is hashed before main effect!");
    if (inverse_mapping.find(src[1]) == inverse_mapping.end()) throw std::logic_error("interaction is hashed before main effect!");
    std::string key(inverse_mapping[src[0]]);
    key.append(":");
    key.append(inverse_mapping[src[1]]);
    #endif
    e[key.c_str()] = Rcpp::wrap((int) retval);
    inverse_mapping[retval] = key;
  } 
  else {
    e[buf] = Rcpp::wrap((int) retval);
    inverse_mapping[retval] = buf;
  }
  return retval;
}

using namespace Rcpp;

//'@export hash.sign
//[[Rcpp::export("hash.sign")]]
IntegerVector xi(CharacterVector src) {
  IntegerVector retval(src.size(), 0);
  for(int i = 0;i < src.size();i++) {
    const char* str = CHAR(src[i]);
    if (::strcmp("(Intercept)", str) == 0) continue;
    retval[i] = (int) PMurHash32(MURMURHASH3_XI_SEED, str, ::strlen(str));
    if (retval[i] < 0) retval[i] = -1;
    else retval[i] = 1;
  }
  return retval;
}

//'@export hashed.value
//[[Rcpp::export("hashed.value")]]
IntegerVector h(CharacterVector src) {
  IntegerVector retval(src.size(), 0);
  for(int i = 0;i < src.size();i++) {
    const char* str = CHAR(src[i]);
    if (::strcmp("(Intercept)", str) == 0) continue;
    retval[i] = PMurHash32(MURMURHASH3_H_SEED, str, ::strlen(str));
  }
  return retval;
}

MH_UINT32 interaction(MH_UINT32 a, MH_UINT32 b) {
  MH_UINT32 buf[2];
#ifdef BOOST_BIG_ENDIAN
  buf[0] = bswap_32(a);
  buf[1] = bswap_32(b);
#else
  buf[0] = a;
  buf[1] = b;
#endif
  return PMurHash32(MURMURHASH3_H_SEED, reinterpret_cast<char*>(buf), sizeof(MH_UINT32) * 2);
}

//'@export hashed.interaction.value
//[[Rcpp::export("hashed.interaction.value")]]
IntegerVector h2(CharacterVector src) {
  IntegerVector retval(src.size(), 0);
  std::vector<std::string> tokens;
  std::deque<MH_UINT32> values;
  for(int i = 0;i < src.size();i++) {
    const char* str = CHAR(src[i]);
    boost::split(tokens, str, boost::is_any_of(":"));
    values.resize(tokens.size());
    for(int j = 0;j < tokens.size();j++) {
      values[j] = PMurHash32(MURMURHASH3_H_SEED, tokens[j].c_str(), tokens[j].size());
    }
    MH_UINT32 value = values[0];
    values.pop_front();
    while(values.size() > 0) {
      value = interaction(value, values[0]);
      values.pop_front();
    }
    retval[i] = value;
  }
  return retval;
}
