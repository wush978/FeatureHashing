#ifndef __HASH_FUNCTION_IMPLEMENTATION_HPP__
#define __HASH_FUNCTION_IMPLEMENTATION_HPP__

#include <hash_function.h>
#include <Rcpp.h>

class NullHashFunction : public HashFunction {
  
  public:
  
  virtual uint32_t operator()(const char* buf, int size, bool is_interaction = false);

};

class MurmurHash3HashFunction : public HashFunction {
  
  uint32_t seed;
  
public :

  MurmurHash3HashFunction(uint32_t _seed) : seed(_seed) { }

  virtual uint32_t operator()(const char* buf, int size, bool is_interaction = false);

};

class MurmurHash3LogHashFunction : public HashFunction {
  
  uint32_t seed;
  Rcpp::Environment e;
  std::map<uint32_t, std::string> inverse_mapping;
  
public:

  MurmurHash3LogHashFunction(SEXP _e, uint32_t _seed) 
  : HashFunction(), seed(_seed), e(_e)
  { }
  
  virtual uint32_t operator()(const char* buf, int size, bool is_interaction = false);
  
};

# endif // __HASH_FUNCTION_IMPLEMENTATION_HPP__