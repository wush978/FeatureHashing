#include <cstring>
#include <memory>
#include <boost/detail/endian.hpp>
#include <Rcpp.h>
#include "digestlocal.h"
#include "tag.h"

#ifdef linux
#include <byteswap.h>
#endif

#ifndef bswap_32
#ifdef __APPLE__
#include <libkern/OSByteOrder.h>
#else
uint32_t bswap_32(uint32_t x) {
  return ((x & 0xff000000) >> 24) |
         ((x & 0x00ff0000) >>  8) |
         ((x & 0x0000ff00) <<  8) |
         ((x & 0x000000ff) << 24);
}
#endif
#endif

using namespace Rcpp;

typedef std::map< std::string, std::string > NameClassMapping;
typedef std::vector< std::string > StrVec;

template<typename DataFrameLike>
NameClassMapping get_class(DataFrameLike data) {
  Function lapply("lapply");
  Function fclass("class");
  List colclass(lapply(data, fclass));
  CharacterVector colname(data.attr("names"));
  SEXP pcolname = wrap(colname);
  NameClassMapping retval;
  for(int i = 0;i < Rf_length(colclass);i++) {
    const char *pname = CHAR(STRING_ELT(pcolname, i));
    StrVec classes(as<StrVec>(colclass[i]));
    if (std::find(classes.begin(), classes.end(), "factor") != classes.end()) {
      retval[pname] = "factor";
    } else {
      retval[pname] = *classes.begin();
    }
  }
  return retval;
}

class HashFunction {

public:

  virtual uint32_t operator()(const char* buf, int size, bool is_interaction = false) = 0;

};

//class CRC32HashFunction : public HashFunction {
//  
//public:
//
//  virtual uint32_t operator()(const char* buf, int size) {
//    return ::FeatureHashing_crc32(buf, size);
//  }
//
//};

class MurmurHash3HashFunction : public HashFunction {
  
  uint32_t seed;
  
public :

  MurmurHash3HashFunction(uint32_t _seed) : seed(_seed) { }

  virtual uint32_t operator()(const char* buf, int size, bool is_interaction = false) {
    return ::PMurHash32(seed, buf, size);
  }
};

//class CRC32LogHashFunction : public HashFunction {
//  
//  Environment e;
//  
//public:
//
//  CRC32LogHashFunction(SEXP _e) 
//  : HashFunction(), e(_e)
//  { }
//  
//  virtual uint32_t operator()(const char* buf, int size) {
//    uint32_t retval = FeatureHashing_crc32(buf, size);
//    e[buf] = wrap(retval);
//    return retval;
//  }
//  
//};

class MurmurHash3LogHashFunction : public HashFunction {
  
  uint32_t seed;
  Environment e;
  std::map<uint32_t, std::string> inverse_mapping;
  
public:

  MurmurHash3LogHashFunction(SEXP _e, uint32_t _seed) 
  : HashFunction(), seed(_seed), e(_e)
  { }
  
  virtual uint32_t operator()(const char* buf, int size, bool is_interaction = false) {
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
      e[key.c_str()] = wrap(retval);
      inverse_mapping[retval] = key;
    } 
    else {
      e[buf] = wrap(retval);
      inverse_mapping[retval] = buf;
    }
    return retval;
  }
  
};

class VectorConverter {

protected:
  std::vector<uint32_t> feature_buffer;
  std::vector<double> value_buffer;
  std::string name;
  size_t name_len;
  HashFunction *h_main, *h_binary;

public:
  
  explicit VectorConverter(const std::string& _name, HashFunction* _h_main, HashFunction* _h_binary) 
  : name(_name), name_len(_name.size()), h_main(_h_main), h_binary(_h_binary)
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
  
  CharacterVector src;
  SEXP psrc;
  
public:

  explicit CharacterConverter(SEXP _src, const std::string& _name, HashFunction* _h_main, HashFunction* _h_binary)
  : VectorConverter(_name, _h_main, _h_binary), src(_src), psrc(wrap(src)) {
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
  
  IntegerVector src;
  CharacterVector levels;
  SEXP plevels;

public: 

  explicit FactorConverter(SEXP _src, const std::string& _name, HashFunction* _h_main, HashFunction* _h_binary) 
  : VectorConverter(_name, _h_main, _h_binary), src(_src), levels(src.attr("levels")), plevels(wrap(levels)) {
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
  
  Vector<RType> src;
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

  explicit DenseConverter(SEXP _src, const std::string& _name, HashFunction* _h_main, HashFunction* _h_binary) 
  : VectorConverter(_name, _h_main, _h_binary), src(_src), value(get_hashed_feature(h_main, "")), 
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
      feature_buffer[0] = value;
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

public:
  
  explicit TagConverter(const std::string& _name, HashFunction* _h_main, HashFunction* _h_binary, const std::string& _delim)
  : VectorConverter(_name, _h_main, _h_binary), delim(_delim), cache_i(-1)
  { }
  
  virtual ~TagConverter() { }
  
  virtual const std::vector<uint32_t>& get_feature(size_t i) {
    get_tags(i);
    feature_buffer.resize(cache_tags.size());
    size_t k = 0;
    for(auto j = cache_tags.begin();j != cache_tags.end();j++) {
      feature_buffer[k++] = get_hashed_feature(h_main, j->c_str());
    }
    return feature_buffer;
  }
  
  virtual const std::vector<double>& get_value(size_t i) {
    get_tags(i);
    value_buffer.resize(cache_tags.size());
    size_t k = 0;
    for(auto j = cache_tags.begin();j != cache_tags.end();j++) {
      value_buffer[k++] = get_sign(get_hashed_feature(h_binary, j->c_str()));
    }
    return value_buffer;
  }
  
};

class TagExistenceFactorConverter : public TagConverter< std::set<std::string> > {
  
  IntegerVector src;
  CharacterVector levels;
  SEXP plevels;
  std::vector<std::string> cache_splitted;

protected:

  virtual void get_tags(size_t i) {
    if (i == cache_i) return;
    if (src[i] == NA_INTEGER) {
      cache_tags.clear();
    } else {
      const char* str = CHAR(STRING_ELT(plevels, src[i] - 1));
      std::vector<std::string> temp(split(str, delim));
      cache_splitted.swap(temp);
      cache_tags.clear();
      cache_tags.insert(cache_splitted.begin(), cache_splitted.end());
    }
  }

public:

  explicit TagExistenceFactorConverter(SEXP _src, const std::string& _name, HashFunction* _h_main, HashFunction* _h_binary, const std::string& _delim)
  : TagConverter< std::set<std::string> >(_name, _h_main, _h_binary, _delim), src(_src), levels(src.attr("levels")), plevels(wrap(levels))
  { }
  
  virtual ~TagExistenceFactorConverter() { }
  
};

class TagExistenceCharacterConverter : public TagConverter< std::set<std::string> > {

  CharacterVector src;
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
      std::vector<std::string> temp(split(str, delim));
      cache_splitted.swap(temp);
      cache_tags.clear();
      cache_tags.insert(cache_splitted.begin(), cache_splitted.end());    
    }
  }

public:

  explicit TagExistenceCharacterConverter(SEXP _src, const std::string& _name, HashFunction* _h_main, HashFunction* _h_binary, const std::string& _delim)
  : TagConverter< std::set<std::string> >(_name, _h_main, _h_binary, _delim), src(_src), psrc(wrap(src))
  { }

  virtual ~TagExistenceCharacterConverter() { }
  
};

class TagCountFactorConverter : public TagConverter< std::vector<std::string> > {

  IntegerVector src;
  CharacterVector levels;
  SEXP plevels;

protected:

  virtual void get_tags(size_t i) {
    if (i == cache_i) return;
    if (src[i] == NA_INTEGER) {
      cache_tags.clear();
    } else {
      const char* str = CHAR(STRING_ELT(plevels, src[i] - 1));
      std::vector<std::string> temp(split(str, delim));
      cache_tags.swap(temp);
    }
  }
  
public:

  explicit TagCountFactorConverter(SEXP _src, const std::string& _name, HashFunction* _h_main, HashFunction* _h_binary, const std::string& _delim)
  : TagConverter< std::vector<std::string> >(_name, _h_main, _h_binary, _delim), src(_src), levels(src.attr("levels")), plevels(wrap(levels))
  { }
  
  virtual ~TagCountFactorConverter() { }
  
};

class TagCountCharacterConverter : public TagConverter< std::vector<std::string> > {

  CharacterVector src;
  SEXP psrc;
  
protected:
  
  virtual void get_tags(size_t i) {
    if (i == cache_i) return;
    SEXP pstr = STRING_ELT(psrc, i);
    if (pstr == NA_STRING) {
      cache_tags.clear();
    } else {
      const char* str = CHAR(STRING_ELT(psrc, i));
      std::vector<std::string> temp(split(str, delim));
      cache_tags.swap(temp);
    }
  }

public:

  explicit TagCountCharacterConverter(SEXP _src, const std::string& _name, HashFunction* _h_main, HashFunction* _h_binary, const std::string& _delim)
  : TagConverter< std::vector<std::string> >(_name, _h_main, _h_binary, _delim), src(_src), psrc(wrap(src))
  { }

  virtual ~TagCountCharacterConverter() { }

};


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

class InteractionConverter : public VectorConverter {
  
  pVectorConverter a, b;

public:

  explicit InteractionConverter(pVectorConverter _a, pVectorConverter _b, HashFunction* _h_main, HashFunction* _h_binary) : 
  VectorConverter("", _h_main, _h_binary), a(_a), b(_b) { }
  
  virtual ~InteractionConverter() { }
  
  virtual const std::vector<uint32_t>& get_feature(size_t i) {
    const std::vector<uint32_t> &afeature_buffer(a->get_feature(i)), &bfeature_buffer(b->get_feature(i));
    feature_buffer.resize(afeature_buffer.size() * bfeature_buffer.size());
    value_buffer.resize(afeature_buffer.size() * bfeature_buffer.size());
    size_t l = 0;
    for(auto j = 0;j < afeature_buffer.size();j++) {
      for(auto k = 0;k < bfeature_buffer.size();k++) {
        feature_buffer[l] = get_hashed_feature(h_main, afeature_buffer[j], bfeature_buffer[k]);
        value_buffer[l] = get_sign(get_hashed_feature(h_binary, afeature_buffer[j], bfeature_buffer[k]));
        l++;
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

typedef std::shared_ptr<InteractionConverter> pInteractionConverter;

template<typename DataFrameLike>
const ConvertersVec get_converters(
  const NameClassMapping& reference_class, RObject tf, DataFrameLike data, HashFunction* _h_main, HashFunction* _h_binary
  ) {
  NumericMatrix tfactors(wrap(tf.attr("factors")));
  CharacterVector reference_name, feature_name;
  Environment feature_hashing(Environment::namespace_env("FeatureHashing"));
  Function parse_tag(feature_hashing["parse_tag"]);
  std::set<int> specials;
  {
    List tmp(tf.attr("specials"));
    SEXP ptag = tmp["tag"];
    if (!Rf_isNull(ptag)) {
      IntegerVector tmpvec(ptag);
      specials.insert(tmpvec.begin(), tmpvec.end());
    }
  }
  {
    List tmp(tfactors.attr("dimnames"));
    reference_name = CharacterVector(tmp[0]); // rownames
    feature_name = CharacterVector(tmp[1]); // colnames
  }
  std::vector< pVectorConverter > retval;
  for(int i = 0;i < feature_name.size();i++) {
    bool is_interaction = false;
    for(int j = 0;j < reference_name.size();j++) {
      if (tfactors(j, i) == 0) continue;
      std::string rname(as<std::string>(reference_name[j]));
      #ifdef NOISY_DEBUG
      Rprintf("%s -> ", rname.c_str());
      #endif
      pVectorConverter p(NULL);
      try{
        if (specials.find(j + 1) == specials.end()) {
          if (reference_class.find(rname) == reference_class.end()) throw std::invalid_argument("Failed to find the column:");
          const std::string& rclass(reference_class.find(rname)->second);
          #ifdef NOISY_DEBUG
          Rprintf("%s\n", rclass.c_str());
          #endif
          if (rclass.compare("factor") == 0) {
            #ifdef NOISY_DEBUG
            Rprintf("Initialize FactorConverter\n");
            #endif
            p.reset(new FactorConverter(wrap(data[rname.c_str()]), rname, _h_main, _h_binary));
          } else if (rclass.compare("numeric") == 0) {
            #ifdef NOISY_DEBUG
            Rprintf("Initialize NumConverter\n");
            #endif
            p.reset(new NumConverter(wrap(data[rname.c_str()]), rname, _h_main, _h_binary));
          } else if (rclass.compare("integer") == 0) {
            #ifdef NOISY_DEBUG
            Rprintf("Initialize IntConverter\n");
            #endif
            p.reset(new IntConverter(wrap(data[rname.c_str()]), rname, _h_main, _h_binary));
          } else if (rclass.compare("logical") == 0) {
            #ifdef NOISY_DEBUG
            Rprintf("Initialize LogicalConverter\n");
            #endif
            p.reset(new LogicalConverter(wrap(data[rname.c_str()]), rname, _h_main, _h_binary));            
          } else if (rclass.compare("character") == 0) {
            #ifdef NOISY_DEBUG
            Rprintf("Initialize CharacterConverter\n");
            #endif
            p.reset(new CharacterConverter(wrap(data[rname.c_str()]), rname, _h_main, _h_binary));
          } else {
            throw std::invalid_argument("Non supported type at name: ");
          }
        } 
        else {
          #ifdef NOISY_DEBUG
          Rprintf(" (parsing tag..) ");
          #endif
          List expression(parse_tag(wrap(rname)));
          rname.assign(as<std::string>(expression["reference_name"]));
          #ifdef NOISY_DEBUG
          Rprintf(" (rname ==> %s) ", rname.c_str());
          #endif
          if (reference_class.find(rname) == reference_class.end()) throw std::invalid_argument("Failed to find the column: ");
          const std::string& rclass(reference_class.find(rname)->second);
          #ifdef NOISY_DEBUG
          Rprintf("%s\n", rclass.c_str());
          #endif
          std::string 
            delim(as<std::string>(expression["split"])), 
            type(as<std::string>(expression["type"]));
          #ifdef NOISY_DEBUG
          Rprintf("delim: %s type: %s\n", delim.c_str(), type.c_str());
          #endif
          if (rclass.compare("factor") == 0) {
            if (type.compare("existence") == 0) {
              #ifdef NOISY_DEBUG
              Rprintf("Initialize TagExistenceFactorConverter\n");
              #endif
              p.reset(new TagExistenceFactorConverter(wrap(data[rname.c_str()]), rname, _h_main, _h_binary, delim));
            } else if (type.compare("count") == 0) {
              #ifdef NOISY_DEBUG
              Rprintf("Initialize TagCountFactorConverter\n");
              #endif
              p.reset(new TagCountFactorConverter(wrap(data[rname.c_str()]), rname, _h_main, _h_binary, delim));
            } else {
              throw std::invalid_argument("Non supported type at name: ");
            }
          } else if (rclass.compare("character") == 0) {
            if (type.compare("existence") == 0) {
              #ifdef NOISY_DEBUG
              Rprintf("Initialize TagExistenceCharacterConverter\n");
              #endif
              p.reset(new TagExistenceCharacterConverter(wrap(data[rname.c_str()]), rname, _h_main, _h_binary, delim));
            } else if (type.compare("count") == 0) {
              #ifdef NOISY_DEBUG
              Rprintf("Initialize TagCountCharacterConverter\n");
              #endif
              p.reset(new TagCountCharacterConverter(wrap(data[rname.c_str()]), rname, _h_main, _h_binary, delim));
            } else {
              throw std::invalid_argument("Non supported type at name: ");
            }
          } else {
            throw std::invalid_argument("Non supported type at name: ");
          }
        }
      } catch(std::invalid_argument& e) {
        std::string message(e.what());
        message.append(rname);
        throw std::invalid_argument(message);
      }
      if (!is_interaction) {
        retval.push_back(p);
        is_interaction = true;
      } else {
        pVectorConverter q(*retval.rbegin());
        *retval.rbegin() = pInteractionConverter(new InteractionConverter(q, p, _h_main, _h_binary));
      }
    }
  }
  return retval;
} 

template<typename DataFrameLike>
SEXP hashed_model_matrix(RObject tf, DataFrameLike data, unsigned long hash_size, bool transpose, S4 retval, bool keep_hashing_mapping) {
  if (hash_size > 4294967296) throw std::invalid_argument("hash_size is too big!");
  NameClassMapping reference_class(get_class(data));
  Environment e(Environment::base_env().new_child(wrap(true)));
  std::shared_ptr<HashFunction> pHF(NULL), pBHF(NULL);
  if (keep_hashing_mapping) {
    pHF.reset(new MurmurHash3LogHashFunction(wrap(e), MURMURHASH3_H_SEED));
  } else {
    pHF.reset(new MurmurHash3HashFunction(MURMURHASH3_H_SEED));
  }
  pBHF.reset(new MurmurHash3HashFunction(MURMURHASH3_XI_SEED));
  ConvertersVec converters(get_converters(reference_class, tf, data, pHF.get(), pBHF.get()));
  #ifdef NOISY_DEBUG
  Rprintf("The size of convertres is %d\n", converters.size());
  #endif
  std::vector<int> ivec, pvec(1, 0);
  std::vector<double> xvec;
  bool is_intercept = as<bool>(tf.attr("intercept"));
  #ifdef NOISY_DEBUG
  Rprintf("nrow(data): %d length(converters): %d\n", data.nrows(), converters.size());
  #endif
  if (transpose) {
    for(auto i = 0;i < data.nrows();i++) {
      if (is_intercept) {
        ivec.push_back(0);
        xvec.push_back(1.0);
      }
      for(auto j = converters.begin();j != converters.end();j++) {
        pVectorConverter& p(*j);
        const std::vector<uint32_t>& i_origin(p->get_feature(i));
        const std::vector<double>& x_origin(p->get_value(i));
        #ifdef NOISY_DEBUG
        std::for_each(i_origin.begin(), i_origin.end(), [&hash_size](uint32_t hashed_value) {
          Rprintf("(%zu module %d = %d),", hashed_value, hash_size, hashed_value % hash_size);
        });
        Rprintf("\n");
        #endif
        std::for_each(i_origin.begin(), i_origin.end(), [&ivec, &xvec, &hash_size](uint32_t hashed_value) {
          ivec.push_back(hashed_value % hash_size);
        });
        xvec.insert(xvec.end(), x_origin.begin(), x_origin.end());
      }
      pvec.push_back(ivec.size());
    }
  }
  else {
    std::map< uint32_t, std::pair< std::vector<int>, std::vector<double> > > cache;
    if (is_intercept) {
      std::pair< std::vector<int>, std::vector<double> >& k(cache[0]);
      k.first.resize(data.nrows());
      for(int i = 0;i < data.nrows();i++) {
        k.first[i] = i;
      }
      k.second.resize(data.nrows(), 1.0);
    }
    for(auto i = 0;i < data.nrows();i++) {
      for(auto j = converters.begin();j != converters.end();j++) {
        pVectorConverter& p(*j);
        const std::vector<uint32_t>& i_origin(p->get_feature(i));
        const std::vector<double>& x_origin(p->get_value(i));
        auto x_value = x_origin.begin();
        std::for_each(i_origin.begin(), i_origin.end(), [&cache, &hash_size, &x_value, &i](uint32_t hashed_value) {
          std::pair< std::vector<int>, std::vector<double> >& k(cache[hashed_value % hash_size]);
          k.first.push_back(i);
          k.second.push_back(*(x_value++));
        });
      }
    }
    int pvec_value = ivec.size();
    for(auto i = cache.begin();i != cache.end();i++) {
      while(pvec.size() <= i->first) pvec.push_back(pvec_value);
      ivec.insert(ivec.end(), i->second.first.begin(), i->second.first.end());
      {
        std::vector<int> tmp;
        i->second.first.swap(tmp);
      }
      xvec.insert(xvec.end(), i->second.second.begin(), i->second.second.end());
      {
        std::vector<double> tmp;
        i->second.second.swap(tmp);
      }
      pvec_value = ivec.size();
    }
    pvec.resize(hash_size + 1, pvec_value);
  }
  retval.slot("i") = wrap(ivec);
  retval.slot("p") = wrap(pvec);
  retval.slot("x") = wrap(xvec);
  IntegerVector dim(2);
  if (transpose) {
    dim[0] = hash_size;
    dim[1] = pvec.size() - 1;
    retval.slot("Dim") = dim;
  }
  else {
    dim[0] = data.nrows();
    dim[1] = hash_size;
    retval.slot("Dim") = dim;
  }
  {
    List dimnames(2);
    dimnames[0] = CharacterVector(0);
    dimnames[1] = CharacterVector(0);
    retval.slot("Dimnames") = dimnames;
  }
  retval.slot("factors") = List();
  retval.attr("mapping") = e;
  return retval;
}

//[[Rcpp::export(".hashed.model.matrix.dataframe")]]
SEXP hashed_model_matrix_dataframe(RObject tf, DataFrame data, unsigned long hash_size, bool transpose, S4 retval, bool keep_hashing_mapping) {
  return hashed_model_matrix<DataFrame>(tf, data, hash_size, transpose, retval, keep_hashing_mapping);
}

