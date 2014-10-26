#include <cstring>
#include <memory>
#include <Rcpp.h>
#include "capi.h"

using namespace Rcpp;

typedef std::map< std::string, std::string > NameClassMapping;
typedef std::vector< std::string > StrVec;

NameClassMapping get_class(DataFrame data) {
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

class VectorConverter {

protected:
  std::vector<uint32_t> feature_buffer;
  std::vector<double> value_buffer;
  std::string name;
  size_t name_len;

public:
  
  explicit VectorConverter(const std::string& _name) : name(_name), name_len(_name.size()) { }
  
  virtual ~VectorConverter() { }
  
  virtual const std::vector<uint32_t>& get_feature(size_t i) = 0;
  
  virtual const std::vector<double>& get_value(size_t i) = 0;
  
private:
  
};

class CharacterConverter : public VectorConverter {
  
  CharacterVector src;
  SEXP psrc;
  
public:

  explicit CharacterConverter(SEXP _src, const std::string& _name)
  : VectorConverter(_name), src(_src), psrc(wrap(src)) {
    value_buffer.resize(1, 1.0);
    feature_buffer.resize(1);
  }
  
  virtual ~CharacterConverter() { }
  
  virtual const std::vector<uint32_t>& get_feature(size_t i) {
    const char* str = CHAR(STRING_ELT(psrc, i));
    name.append(str);
    feature_buffer[0] = FeatureHashing_crc32(name.c_str(), name.size());
    name.resize(name_len);
    return feature_buffer;
  }

  virtual const std::vector<double>& get_value(size_t i) {
    return value_buffer;
  }

};

class FactorConverter : public VectorConverter {
  
  IntegerVector src;
  CharacterVector levels;
  SEXP plevels;

public: 

  explicit FactorConverter(SEXP _src, const std::string& _name) 
  : VectorConverter(_name), src(_src), levels(src.attr("levels")), plevels(wrap(levels)) {
    value_buffer.resize(1, 1);
    feature_buffer.resize(1);
  }
  
  virtual ~FactorConverter() { }
  
  virtual const std::vector<uint32_t>& get_feature(size_t i) {
    const char* str = CHAR(STRING_ELT(plevels, src[i] - 1)); // R start from 1 and C start from 0
    name.append(str);
    feature_buffer[0] = FeatureHashing_crc32(name.c_str(), name.size());
    name.resize(name_len);
    return feature_buffer;
  }
  
  virtual const std::vector<double>& get_value(size_t i) {
    return value_buffer;
  }

};

template<typename ValueType, int RType>
class DenseConverter : public VectorConverter {
  
  Vector<RType> src;
  
public:

  explicit DenseConverter(SEXP _src, const std::string& _name) : VectorConverter(_name), src(_src) {
    feature_buffer.resize(1, FeatureHashing_crc32(name.c_str(), name.size()));
    value_buffer.resize(1, 0.0);
  }
  
  virtual ~DenseConverter() { }
  
  virtual const std::vector<uint32_t>& get_feature(size_t i) {
    return feature_buffer;
  }
  
  virtual const std::vector<double>& get_value(size_t i) {
    value_buffer[0] = src[i];
    return value_buffer;
  }
  
};

typedef std::shared_ptr<VectorConverter> pVectorConverter;
typedef std::shared_ptr<CharacterConverter> pCharacterConverter;
typedef std::shared_ptr<FactorConverter> pFactorConverter;
typedef DenseConverter<double, REALSXP> NumConverter;
typedef std::shared_ptr<NumConverter> pNumConverter;
typedef DenseConverter<int, INTSXP> IntConverter;
typedef std::shared_ptr<IntConverter> pIntConverter;
typedef std::vector< pVectorConverter > ConvertersVec;

class InteractionConverter : public VectorConverter {
  
  pVectorConverter a, b;

public:

  explicit InteractionConverter(pVectorConverter _a, pVectorConverter _b) : VectorConverter(""), a(_a), b(_b) { }
  
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

const ConvertersVec get_converters(
  const NameClassMapping& reference_class, RObject tf, DataFrame data
  ) {
  NumericMatrix tfactors(wrap(tf.attr("factors")));
  CharacterVector reference_name, feature_name;
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
      const std::string& rclass(reference_class.find(rname)->second);
      #ifdef NOISY_DEBUG
      Rprintf("%s\n", rclass.c_str());
      #endif
      pVectorConverter p(NULL);
      if (rclass.compare("factor") == 0) {
        #ifdef NOISY_DEBUG
        Rprintf("Initialize FactorConverter\n");
        #endif
        p.reset(new FactorConverter(wrap(data[rname.c_str()]), rname));
      } else if (rclass.compare("numeric") == 0) {
        #ifdef NOISY_DEBUG
        Rprintf("Initialize NumConverter\n");
        #endif
        p.reset(new NumConverter(wrap(data[rname.c_str()]), rname));
      } else if (rclass.compare("integer") == 0) {
        #ifdef NOISY_DEBUG
        Rprintf("Initialize IntConverter\n");
        #endif
        p.reset(new IntConverter(wrap(data[rname.c_str()]), rname));
      } else if (rclass.compare("character") == 0) {
        #ifdef NOISY_DEBUG
        Rprintf("Initialize CharacterConverter\n");
        #endif
        p.reset(new CharacterConverter(wrap(data[rname.c_str()]), rname));
      } else {
        std::string message("Non supported type: ");
        message.append(rclass);
        message.append(" at name: ");
        message.append(rname);
        throw std::invalid_argument(message);
      }
      if (!is_interaction) {
        retval.push_back(p);
        is_interaction = true;
      } else {
        pVectorConverter q(*retval.rbegin());
        *retval.rbegin() = pInteractionConverter(new InteractionConverter(q, p));
      }
    }
  }
  return retval;
} 

//[[Rcpp::export(".hashed.model.matrix")]]
SEXP hashed_model_matrix(RObject tf, DataFrame data, unsigned long hash_size, S4 retval) {
  if (hash_size > 4294967296) throw std::invalid_argument("hash_size is too big!");
  NameClassMapping reference_class(get_class(data));
  ConvertersVec converters(get_converters(reference_class, tf, data));
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
  return retval;
}