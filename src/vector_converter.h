/*
 * This file is part of FeatureHashing
 * Copyright (C) 2015 Wush Wu
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VECTOR_CONVERTER_HPP__
#define __VECTOR_CONVERTER_HPP__

#include "hash_function.h"
#include "split.h"
#include <Rcpp.h>

struct VectorConverterParam;
class VectorConverter;
class CharacterConverter;
class FactorConverter;
template<typename ValueType, int RType>
class DenseConverter;
class TagExistenceFactorConverter;
class TagExistenceCharacterConverter;
class TagCountFactorConverter;
class TagCountCharacterConverter;
class InteractionConverter;

typedef VectorConverterParam Param;
typedef std::shared_ptr<VectorConverter> pVectorConverter;
typedef std::shared_ptr<CharacterConverter> pCharacterConverter;
typedef std::shared_ptr<FactorConverter> pFactorConverter;
typedef DenseConverter<double, REALSXP> NumConverter;
typedef std::shared_ptr<NumConverter> pNumConverter;
typedef DenseConverter<int, INTSXP> IntConverter;
typedef std::shared_ptr<IntConverter> pIntConverter;
typedef DenseConverter<int, LGLSXP> LogicalConverter;
typedef std::shared_ptr<LogicalConverter> pLogicalConverter;
typedef std::shared_ptr<TagExistenceFactorConverter> pTagExistenceFactorConverter;
typedef std::shared_ptr<TagExistenceCharacterConverter> pTagExistenceCharacterConverter;
typedef std::shared_ptr<TagCountFactorConverter> pTagCountFactorConverter;
typedef std::shared_ptr<TagCountCharacterConverter> pTagCountCharacterConverter;
typedef std::vector< pVectorConverter > ConvertersVec;
typedef std::shared_ptr<InteractionConverter> pInteractionConverter;

/**
 * Paramter of initializing VectorConverter
 */
struct VectorConverterParam {
  
  std::string name;
  HashFunction* h_main;
  HashFunction* h_binary;
  size_t hash_size;

  VectorConverterParam(const std::string& _name, HashFunction* _h_main, HashFunction* _h_binary, size_t _hash_size)
  : name(_name), h_main(_h_main), h_binary(_h_binary), hash_size(_hash_size) 
  { }
  
};

class VectorConverter {

protected:
  std::vector<uint32_t> feature_buffer;
  std::vector<double> value_buffer;
  std::string name;
  size_t name_len;
  HashFunction *h_main, *h_binary;
  size_t hash_size;

public:

  bool is_final;
  
  explicit VectorConverter(const Param& param) 
  : name(param.name), name_len(param.name.size()), h_main(param.h_main), 
  h_binary(param.h_binary), hash_size(param.hash_size), is_final(true)
  { }
  
  virtual ~VectorConverter() { }
  
  /**
   * Evaluate the hashed feature from the raw data.
   * The \code{get_feature} should be called before \code{get_value}
   */
  virtual const std::vector<uint32_t>& get_feature(size_t i) = 0;
  
  /**
   * Evaluate the value from the raw data
   * The \code{get_feature} should be called before \code{get_value}
   */ 
  virtual const std::vector<double>& get_value(size_t i) = 0;
  
  const std::string& get_name() const {
    return name;
  }
  
protected:

  /**
   * @return -1 if (int) v < 0, or +1 otherwise
   */ 
  static inline int get_sign(uint32_t v) {
    if ((int) v < 0) return -1;
    else return 1;
  }

  uint32_t get_hashed_feature(HashFunction* h, const char* str) {
    name.append(str);
    #ifdef NOISY_DEBUG
    Rprintf("hashing %s ... ", name.c_str());
    #endif
    uint32_t retval = (*h)(name.c_str(), name.size());
    #ifdef NOISY_DEBUG
    Rprintf(" got %zu \n", retval);
    #endif
    name.resize(name_len);
    return retval;
  }

};

class CharacterConverter : public VectorConverter {
  
  Rcpp::CharacterVector src;
  SEXP psrc;
  
public:

  explicit CharacterConverter(SEXP _src, const Param& param)
  : VectorConverter(param), src(_src), psrc(wrap(src)) {
    value_buffer.reserve(1);
    feature_buffer.reserve(1);
  }
  
  virtual ~CharacterConverter() { }

  virtual const std::vector<uint32_t>& get_feature(size_t i) {
    SEXP pstr = STRING_ELT(psrc, i);
    if (pstr == NA_STRING) {
      feature_buffer.clear();
    } else {
      const char* str = CHAR(pstr);
      feature_buffer.resize(1);
      feature_buffer[0] = get_hashed_feature(h_main, str);
      if (is_final) feature_buffer[0] = feature_buffer[0] % hash_size;
    }
    return feature_buffer;
  }

  virtual const std::vector<double>& get_value(size_t i) {
    SEXP pstr = STRING_ELT(psrc, i);
    if (pstr == NA_STRING) {
      value_buffer.clear();
    } else {
      const char* str = CHAR(pstr);
      uint32_t sign_value = get_hashed_feature(h_binary, str);
      value_buffer.resize(1);
      value_buffer[0] = get_sign(sign_value);
    }
    return value_buffer;
  }
};

class FactorConverter : public VectorConverter {
  
  Rcpp::IntegerVector src;
  Rcpp::CharacterVector levels;
  SEXP plevels;

public: 

  explicit FactorConverter(SEXP _src, const Param& param) 
  : VectorConverter(param), src(_src), levels(src.attr("levels")), plevels(wrap(levels)) {
    value_buffer.reserve(1);
    feature_buffer.reserve(1);
  }
  
  virtual ~FactorConverter() { }
  
  virtual const std::vector<uint32_t>& get_feature(size_t i) {
    if (src[i] == NA_INTEGER) {
      feature_buffer.clear();
    } else {
      feature_buffer.resize(1);
      const char* str = CHAR(STRING_ELT(plevels, src[i] - 1)); // R start from 1 and C start from 0
      feature_buffer[0] = get_hashed_feature(h_main, str);
      if (is_final) feature_buffer[0] = feature_buffer[0] % hash_size;
    }
    return feature_buffer;
  }
  
  virtual const std::vector<double>& get_value(size_t i) {
    if (src[i] == NA_INTEGER) {
      value_buffer.clear();
    } else {
      const char* str = CHAR(STRING_ELT(plevels, src[i] - 1)); // R start from 1 and C start from 0
      uint32_t sign_value = get_hashed_feature(h_binary, str);
      value_buffer.resize(1);
      value_buffer[0] = get_sign(sign_value);
    }
    return value_buffer;
  }

};

template<typename ValueType, int RType>
class DenseConverter : public VectorConverter {
  
  Rcpp::Vector<RType> src;
  uint32_t value;
  int sign_value;
  
  static bool isNA(ValueType x) {
    switch(RType) {
    case REALSXP: 
      return R_IsNA(x);
    case INTSXP:
      return x == NA_INTEGER;
    case LGLSXP:
      return x == NA_LOGICAL;
    default:
      throw std::logic_error("Invalid RType");
    }
  }
  
public:

  explicit DenseConverter(SEXP _src, const Param& param) 
  : VectorConverter(param), src(_src), value(get_hashed_feature(h_main, "")), 
  sign_value(get_sign(get_hashed_feature(h_binary, ""))) {
    feature_buffer.reserve(1);
    value_buffer.reserve(1);
  }
  
  virtual ~DenseConverter() { }
  
  virtual const std::vector<uint32_t>& get_feature(size_t i) {
    if (isNA(src[i]) | src[i] == 0) {
      feature_buffer.clear();
    } else {
      feature_buffer.resize(1);
      feature_buffer[0] = (is_final ? value % hash_size : value);
    }
    return feature_buffer;
  }
  
  virtual const std::vector<double>& get_value(size_t i) {
    if (isNA(src[i]) | src[i] == 0) {
      value_buffer.clear();
    } else {
      value_buffer.resize(1);
      value_buffer[0] = sign_value * src[i];
    }
    return value_buffer;
  }
  
};

template<typename CacheTagType>
class TagConverter : public VectorConverter {
  

protected:
  
  std::string delim;
  size_t cache_i;
  CacheTagType cache_tags;
  
  virtual void get_tags(size_t i) = 0;
  
  std::vector<std::string> split_tags(const std::string& src) {
    std::vector<std::string> temp(split(src, delim));
    temp.erase(std::remove(temp.begin(), temp.end(), ""), temp.end());
    return temp;
  }
  
  virtual void decollision_feature(size_t i) { }
  
  virtual void decollision_value(size_t i) { }

public:
  
  explicit TagConverter(const Param& param, const std::string& _delim)
  : VectorConverter(param), delim(_delim), cache_i(-1)
  { }
  
  virtual ~TagConverter() { }
  
  virtual const std::vector<uint32_t>& get_feature(size_t i) {
    get_tags(i);
    feature_buffer.resize(cache_tags.size());
    size_t k = 0;
    for(auto j = cache_tags.begin();j != cache_tags.end();j++) {
      feature_buffer[k++] = (is_final ? get_hashed_feature(h_main, j->c_str()) % hash_size : get_hashed_feature(h_main, j->c_str()));
    }
    if (is_final) decollision_feature(i);
    return feature_buffer;
  }
  
  virtual const std::vector<double>& get_value(size_t i) {
    get_tags(i);
    value_buffer.resize(cache_tags.size());
    size_t k = 0;
    for(auto j = cache_tags.begin();j != cache_tags.end();j++) {
      value_buffer[k++] = get_sign(get_hashed_feature(h_binary, j->c_str()));
    }
    if (is_final) decollision_value(i);
    return value_buffer;
  }
  
};

class TagExistenceConverter : public TagConverter< std::set<std::string> > { 
  
  size_t decollision_mark;

protected:

  virtual void decollision_feature(size_t i) {
    std::set<uint32_t> temp;
    temp.insert(feature_buffer.begin(), feature_buffer.end());
    feature_buffer.clear();
    feature_buffer.assign(temp.begin(), temp.end());
    decollision_mark = i + 1;
  }
  
  virtual void decollision_value(size_t i) {
    if (decollision_mark != i + 1) throw std::logic_error("The order of decollision is unexpected");
    value_buffer.resize(feature_buffer.size());
  }
  
public:

  TagExistenceConverter(const Param& param, const std::string& _delim)
  : TagConverter<std::set<std::string> >(param, _delim), decollision_mark(0)
  { }
  
  virtual ~TagExistenceConverter() { }
  
};

class TagExistenceFactorConverter : public TagExistenceConverter {
  
  Rcpp::IntegerVector src;
  Rcpp::CharacterVector levels;
  SEXP plevels;
  std::vector<std::string> cache_splitted;

protected:

  virtual void get_tags(size_t i) {
    if (i == cache_i) return;
    if (src[i] == NA_INTEGER) {
      cache_tags.clear();
    } else {
      const char* str = CHAR(STRING_ELT(plevels, src[i] - 1));
      std::vector<std::string> temp(split_tags(str));
      cache_splitted.swap(temp);
      cache_tags.clear();
      cache_tags.insert(cache_splitted.begin(), cache_splitted.end());
    }
  }

public:

  explicit TagExistenceFactorConverter(SEXP _src, const Param& param, const std::string& _delim)
  : TagExistenceConverter(param, _delim), src(_src), levels(src.attr("levels")), plevels(wrap(levels))
  { }
  
  virtual ~TagExistenceFactorConverter() { }
  
};

class TagExistenceCharacterConverter : public TagExistenceConverter {

  Rcpp::CharacterVector src;
  SEXP psrc;
  std::vector<std::string> cache_splitted;

protected:

  virtual void get_tags(size_t i) {
    if (i == cache_i) return;
    SEXP pstr = STRING_ELT(psrc, i);
    if (pstr == NA_STRING) {
      cache_tags.clear();
    } else {
      const char* str = CHAR(STRING_ELT(psrc, i));
      std::vector<std::string> temp(split_tags(str));
      cache_splitted.swap(temp);
      cache_tags.clear();
      cache_tags.insert(cache_splitted.begin(), cache_splitted.end());    
    }
  }

public:

  explicit TagExistenceCharacterConverter(SEXP _src, const Param& param, const std::string& _delim)
  : TagExistenceConverter(param, _delim), src(_src), psrc(wrap(src))
  { }

  virtual ~TagExistenceCharacterConverter() { }
  
};

class TagCountFactorConverter : public TagConverter< std::vector<std::string> > {

  Rcpp::IntegerVector src;
  Rcpp::CharacterVector levels;
  SEXP plevels;

protected:

  virtual void get_tags(size_t i) {
    if (i == cache_i) return;
    if (src[i] == NA_INTEGER) {
      cache_tags.clear();
    } else {
      const char* str = CHAR(STRING_ELT(plevels, src[i] - 1));
      std::vector<std::string> temp(split_tags(str));
      cache_tags.swap(temp);
    }
  }
  
public:

  explicit TagCountFactorConverter(SEXP _src, const Param& param, const std::string& _delim)
  : TagConverter< std::vector<std::string> >(param, _delim), src(_src), levels(src.attr("levels")), plevels(wrap(levels))
  { }
  
  virtual ~TagCountFactorConverter() { }
  
};

class TagCountCharacterConverter : public TagConverter< std::vector<std::string> > {

  Rcpp::CharacterVector src;
  SEXP psrc;
  
protected:
  
  virtual void get_tags(size_t i) {
    if (i == cache_i) return;
    SEXP pstr = STRING_ELT(psrc, i);
    if (pstr == NA_STRING) {
      cache_tags.clear();
    } else {
      const char* str = CHAR(STRING_ELT(psrc, i));
      std::vector<std::string> temp(split_tags(str));
      cache_tags.swap(temp);
    }
  }

public:

  explicit TagCountCharacterConverter(SEXP _src, const Param& param, const std::string& _delim)
  : TagConverter< std::vector<std::string> >(param, _delim), src(_src), psrc(wrap(src))
  { }

  virtual ~TagCountCharacterConverter() { }

};

class InteractionConverter : public VectorConverter {
  
  pVectorConverter a, b;

public:

  explicit InteractionConverter(pVectorConverter _a, pVectorConverter _b, const Param& param) : 
  VectorConverter(param), a(_a), b(_b) {
    a->is_final = false;
    b->is_final = false;
  }
  
  virtual ~InteractionConverter() { }
  
  virtual const std::vector<uint32_t>& get_feature(size_t i) {
    const std::vector<uint32_t> &afeature_buffer(a->get_feature(i)), &bfeature_buffer(b->get_feature(i));
    feature_buffer.resize(afeature_buffer.size() * bfeature_buffer.size());
    value_buffer.resize(afeature_buffer.size() * bfeature_buffer.size());
    size_t l = 0;
    if (is_final) {
      for(auto j = 0;j < afeature_buffer.size();j++) {
        for(auto k = 0;k < bfeature_buffer.size();k++) {
          feature_buffer[l] = get_hashed_feature(h_main, afeature_buffer[j], bfeature_buffer[k]) % hash_size;
          value_buffer[l] = get_sign(get_hashed_feature(h_binary, afeature_buffer[j], bfeature_buffer[k]));
          l++;
        }
      }
    } else {
      for(auto j = 0;j < afeature_buffer.size();j++) {
        for(auto k = 0;k < bfeature_buffer.size();k++) {
          feature_buffer[l] = get_hashed_feature(h_main, afeature_buffer[j], bfeature_buffer[k]);
          value_buffer[l] = get_sign(get_hashed_feature(h_binary, afeature_buffer[j], bfeature_buffer[k]));
          l++;
        }
      }
    }
    return feature_buffer;
  }
  
  virtual const std::vector<double>& get_value(size_t i) {
    const std::vector<double> &avalue_buffer(a->get_value(i)), &bvalue_buffer(b->get_value(i));
    size_t l = 0;
    for(auto j = 0;j < avalue_buffer.size();j++) {
      for(auto k = 0;k < bvalue_buffer.size();k++) {
        value_buffer[l] = avalue_buffer[j] * bvalue_buffer[k] * value_buffer[l];
        l++;
      }
    }
    return value_buffer;
  }
  
private:

  uint32_t get_hashed_feature(HashFunction *h, uint32_t a, uint32_t b) {
    uint32_t buf[2];
    #ifdef BOOST_BIG_ENDIAN
    buf[0] = bswap_32(a);
    buf[1] = bswap_32(b);
    #else
    buf[0] = a;
    buf[1] = b;
    #endif
    return (*h)(reinterpret_cast<char*>(buf), sizeof(uint32_t) * 2, true);
  }
  
};

#endif // __VECTOR_CONVERTER_HPP__