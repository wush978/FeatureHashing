#include <cstring>
#include <memory>
#include <Rcpp.h>
#include "capi.h"
#include "tag.hpp"

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

  virtual uint32_t operator()(const char* buf, int size) = 0;

};

class CRC32HashFunction : public HashFunction {
  
public:

  virtual uint32_t operator()(const char* buf, int size) {
    return ::FeatureHashing_crc32(buf, size);
  }

};

class CRC32LogHashFunction : public CRC32HashFunction {
  
  Environment e;
  
public:

  CRC32LogHashFunction(SEXP _e) 
  : CRC32HashFunction(), e(_e)
  { }
  
  virtual uint32_t operator()(const char* buf, int size) {
    uint32_t retval = FeatureHashing_crc32(buf, size);
    e[buf] = wrap(retval);
    return retval;
  }
  
};

class VectorConverter {

protected:
  std::vector<uint32_t> feature_buffer;
  std::vector<double> value_buffer;
  std::string name;
  size_t name_len;
  HashFunction* h;

public:
  
  explicit VectorConverter(const std::string& _name, HashFunction* _h) 
  : name(_name), name_len(_name.size()), h(_h)
  { }
  
  virtual ~VectorConverter() { }
  
  virtual const std::vector<uint32_t>& get_feature(size_t i) = 0;
  
  virtual const std::vector<double>& get_value(size_t i) = 0;
  
  const std::string& get_name() const {
    return name;
  }
  
private:
  
};

class CharacterConverter : public VectorConverter {
  
  CharacterVector src;
  SEXP psrc;
  
public:

  explicit CharacterConverter(SEXP _src, const std::string& _name, HashFunction* _h)
  : VectorConverter(_name, _h), src(_src), psrc(wrap(src)) {
    value_buffer.resize(1, 1.0);
    feature_buffer.resize(1);
  }
  
  virtual ~CharacterConverter() { }
  
  virtual const std::vector<uint32_t>& get_feature(size_t i) {
    SEXP pstr = STRING_ELT(psrc, i);
    if (pstr == NA_STRING) {
      feature_buffer.clear();
    } else {
      const char* str = CHAR(pstr);
      feature_buffer.resize(1, 1);
      name.append(str);
      feature_buffer[0] = (*h)(name.c_str(), name.size());
      name.resize(name_len);
    }
    return feature_buffer;
  }

  virtual const std::vector<double>& get_value(size_t i) {
    SEXP pstr = STRING_ELT(psrc, i);
    if (pstr == NA_STRING) {
      value_buffer.clear();
    } else {
      value_buffer.resize(1, 1);
    }
    return value_buffer;
  }

};

class FactorConverter : public VectorConverter {
  
  IntegerVector src;
  CharacterVector levels;
  SEXP plevels;

public: 

  explicit FactorConverter(SEXP _src, const std::string& _name, HashFunction* _h) 
  : VectorConverter(_name, _h), src(_src), levels(src.attr("levels")), plevels(wrap(levels)) {
    value_buffer.resize(1, 1);
    feature_buffer.resize(1);
  }
  
  virtual ~FactorConverter() { }
  
  virtual const std::vector<uint32_t>& get_feature(size_t i) {
    if (src[i] == NA_INTEGER) {
      feature_buffer.clear();
    } else {
      feature_buffer.resize(1);
      const char* str = CHAR(STRING_ELT(plevels, src[i] - 1)); // R start from 1 and C start from 0
      name.append(str);
      feature_buffer[0] = (*h)(name.c_str(), name.size());
      name.resize(name_len);
    }
    return feature_buffer;
  }
  
  virtual const std::vector<double>& get_value(size_t i) {
    if (src[i] == NA_INTEGER) {
      value_buffer.clear();
    } else {
      value_buffer.resize(1, 1);
    }
    return value_buffer;
  }

};

template<typename ValueType, int RType>
class DenseConverter : public VectorConverter {
  
  Vector<RType> src;
  uint32_t value;
  
  static ValueType get_NA() {
    switch(RType) {
    case REALSXP: 
      return NA_REAL;
    case INTSXP:
      return NA_INTEGER;
    case LGLSXP:
      return NA_LOGICAL;
    default:
      throw std::logic_error("Invalid RType");
    }
  }
  
public:

  explicit DenseConverter(SEXP _src, const std::string& _name, HashFunction* _h) 
  : VectorConverter(_name, _h), src(_src), value((*h)(name.c_str(), name.size())) {
    feature_buffer.resize(1, value);
    value_buffer.resize(1, 0.0);
  }
  
  virtual ~DenseConverter() { }
  
  virtual const std::vector<uint32_t>& get_feature(size_t i) {
    if (src[i] == get_NA()) {
      feature_buffer.clear();
    } else {
      feature_buffer.resize(1, value);
    }
    return feature_buffer;
  }
  
  virtual const std::vector<double>& get_value(size_t i) {
    if (src[i] == get_NA()) {
      value_buffer.clear();
    } else {
      value_buffer.resize(1, 0);
    }
    value_buffer[0] = src[i];
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
  
  explicit TagConverter(const std::string& _name, HashFunction* _h, const std::string& _delim)
  : VectorConverter(_name, _h), delim(_delim), cache_i(-1)
  { }
  
  virtual ~TagConverter() { }
  
  virtual const std::vector<uint32_t>& get_feature(size_t i) {
    get_tags(i);
    feature_buffer.resize(cache_tags.size());
    size_t k = 0;
    for(auto j = cache_tags.begin();j != cache_tags.end();j++) {
      name.append(*j);
      feature_buffer[k++] = (*h)(name.c_str(), name.size());
      name.resize(name_len);    
    }
    return feature_buffer;
  }
  
  virtual const std::vector<double>& get_value(size_t i) {
    get_tags(i);
    value_buffer.clear();
    value_buffer.resize(cache_tags.size(), 1);
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

  explicit TagExistenceFactorConverter(SEXP _src, const std::string& _name, HashFunction* _h, const std::string& _delim)
  : TagConverter< std::set<std::string> >(_name, _h, _delim), src(_src), levels(src.attr("levels")), plevels(wrap(levels))
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

  explicit TagExistenceCharacterConverter(SEXP _src, const std::string& _name, HashFunction* _h, const std::string& _delim)
  : TagConverter< std::set<std::string> >(_name, _h, _delim), src(_src), psrc(wrap(src))
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

  explicit TagCountFactorConverter(SEXP _src, const std::string& _name, HashFunction* _h, const std::string& _delim)
  : TagConverter< std::vector<std::string> >(_name, _h, _delim), src(_src), levels(src.attr("levels")), plevels(wrap(levels))
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

  explicit TagCountCharacterConverter(SEXP _src, const std::string& _name, HashFunction* _h, const std::string& _delim)
  : TagConverter< std::vector<std::string> >(_name, _h, _delim), src(_src), psrc(wrap(src))
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
typedef std::shared_ptr<TagExistenceFactorConverter> pTagExistenceFactorConverter;
typedef std::shared_ptr<TagExistenceCharacterConverter> pTagExistenceCharacterConverter;
typedef std::shared_ptr<TagCountFactorConverter> pTagCountFactorConverter;
typedef std::shared_ptr<TagCountCharacterConverter> pTagCountCharacterConverter;
typedef std::vector< pVectorConverter > ConvertersVec;

class InteractionConverter : public VectorConverter {
  
  pVectorConverter a, b;

public:

  explicit InteractionConverter(pVectorConverter _a, pVectorConverter _b, HashFunction* _h) : 
  VectorConverter("", _h), a(_a), b(_b) { }
  
  virtual ~InteractionConverter() { }
  
  virtual const std::vector<uint32_t>& get_feature(size_t i) {
    const std::vector<uint32_t> &afeature_buffer(a->get_feature(i)), &bfeature_buffer(b->get_feature(i));
    feature_buffer.resize(afeature_buffer.size() * bfeature_buffer.size());
    size_t l = 0;
    for(auto j = 0;j < afeature_buffer.size();j++) {
      for(auto k = 0;k < bfeature_buffer.size();k++) {
        feature_buffer[l++] = afeature_buffer[j] ^ bfeature_buffer[k];
      }
    }
    return feature_buffer;
  }
  
  virtual const std::vector<double>& get_value(size_t i) {
    const std::vector<double> &avalue_buffer(a->get_value(i)), &bvalue_buffer(b->get_value(i));
    value_buffer.resize(avalue_buffer.size() * bvalue_buffer.size());
    size_t l = 0;
    for(auto j = 0;j < avalue_buffer.size();j++) {
      for(auto k = 0;k < bvalue_buffer.size();k++) {
        value_buffer[l++] = avalue_buffer[j] * bvalue_buffer[k];
      }
    }
    return value_buffer;
  }
  
};

typedef std::shared_ptr<InteractionConverter> pInteractionConverter;

template<typename DataFrameLike>
const ConvertersVec get_converters(
  const NameClassMapping& reference_class, RObject tf, DataFrameLike data, HashFunction* _h
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
      if (tfactors(i, j) == 0) continue;
      std::string rname(as<std::string>(reference_name[j]));
      #ifdef NOISY_DEBUG
      Rprintf("%s -> ", rname.c_str());
      #endif
      pVectorConverter p(NULL);
      try{
        if (specials.find(j + 1) == specials.end()) {
          const std::string& rclass(reference_class.find(rname)->second);
          #ifdef NOISY_DEBUG
          Rprintf("%s\n", rclass.c_str());
          #endif
          if (rclass.compare("factor") == 0) {
            #ifdef NOISY_DEBUG
            Rprintf("Initialize FactorConverter\n");
            #endif
            p.reset(new FactorConverter(wrap(data[rname.c_str()]), rname, _h));
          } else if (rclass.compare("numeric") == 0) {
            #ifdef NOISY_DEBUG
            Rprintf("Initialize NumConverter\n");
            #endif
            p.reset(new NumConverter(wrap(data[rname.c_str()]), rname, _h));
          } else if (rclass.compare("integer") == 0) {
            #ifdef NOISY_DEBUG
            Rprintf("Initialize IntConverter\n");
            #endif
            p.reset(new IntConverter(wrap(data[rname.c_str()]), rname, _h));
          } else if (rclass.compare("character") == 0) {
            #ifdef NOISY_DEBUG
            Rprintf("Initialize CharacterConverter\n");
            #endif
            p.reset(new CharacterConverter(wrap(data[rname.c_str()]), rname, _h));
          } else {
            throw std::invalid_argument("");
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
              p.reset(new TagExistenceFactorConverter(wrap(data[rname.c_str()]), rname, _h, delim));
            } else if (type.compare("count") == 0) {
              #ifdef NOISY_DEBUG
              Rprintf("Initialize TagCountFactorConverter\n");
              #endif
              p.reset(new TagCountFactorConverter(wrap(data[rname.c_str()]), rname, _h, delim));
            } else {
              throw std::invalid_argument("");
            }
          } else if (rclass.compare("character") == 0) {
            if (type.compare("existence") == 0) {
              #ifdef NOISY_DEBUG
              Rprintf("Initialize TagExistenceCharacterConverter\n");
              #endif
              p.reset(new TagExistenceCharacterConverter(wrap(data[rname.c_str()]), rname, _h, delim));
            } else if (type.compare("count") == 0) {
              #ifdef NOISY_DEBUG
              Rprintf("Initialize TagCountCharacterConverter\n");
              #endif
              p.reset(new TagCountCharacterConverter(wrap(data[rname.c_str()]), rname, _h, delim));
            } else {
              throw std::invalid_argument("");
            }
          } else {
            throw std::invalid_argument("");
          }
        }
      } catch(std::invalid_argument& e) {
        std::string message("Non supported type at name: ");
        message.append(rname);
        throw std::invalid_argument(message);
      }
      if (!is_interaction) {
        retval.push_back(p);
        is_interaction = true;
      } else {
        pVectorConverter q(*retval.rbegin());
        *retval.rbegin() = pInteractionConverter(new InteractionConverter(q, p, _h));
      }
    }
  }
  return retval;
} 

template<typename DataFrameLike>
SEXP hashed_model_matrix(RObject tf, DataFrameLike data, unsigned long hash_size, S4 retval, bool keep_hashing_mapping) {
  if (hash_size > 4294967296) throw std::invalid_argument("hash_size is too big!");
  NameClassMapping reference_class(get_class(data));
  Environment e(Environment::base_env().new_child(wrap(true)));
  std::auto_ptr<HashFunction> pHF(NULL);
  if (keep_hashing_mapping) {
    pHF.reset(new CRC32LogHashFunction(wrap(e)));
  } else {
    pHF.reset(new CRC32HashFunction());
  }
  ConvertersVec converters(get_converters(reference_class, tf, data, pHF.get()));
  #ifdef NOISY_DEBUG
  Rprintf("The size of convertres is %d\n", converters.size());
  #endif
  std::vector<int> ivec, pvec(1, 0);
  std::vector<double> xvec;
  bool is_intercept = as<bool>(tf.attr("intercept"));
  #ifdef NOISY_DEBUG
  Rprintf("nrow(data): %d length(converters): %d\n", data.nrows(), converters.size());
  #endif
  for(auto i = 0;i < data.nrows();i++) {
    if (is_intercept) {
      ivec.push_back(0);
      xvec.push_back(1.0);
    }
    for(auto j = converters.begin();j != converters.end();j++) {
      pVectorConverter& p(*j);
      const std::vector<uint32_t>& i_origin(p->get_feature(i));
      const std::vector<double>& x_origin(p->get_value(i));
      std::for_each(i_origin.begin(), i_origin.end(), [&ivec, &xvec, &hash_size](uint32_t hashed_value) {
        ivec.push_back(hashed_value % hash_size);
      });
      xvec.insert(xvec.end(), x_origin.begin(), x_origin.end());
    }
    pvec.push_back(ivec.size());
  }
  retval.slot("i") = wrap(ivec);
  retval.slot("p") = wrap(pvec);
  retval.slot("x") = wrap(xvec);
  {
    IntegerVector dim(2);
    dim[0] = hash_size;
    dim[1] = pvec.size() - 1;
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
SEXP hashed_model_matrix_dataframe(RObject tf, DataFrame data, unsigned long hash_size, S4 retval, bool keep_hashing_mapping) {
  return hashed_model_matrix<DataFrame>(tf, data, hash_size, retval, keep_hashing_mapping);
}

