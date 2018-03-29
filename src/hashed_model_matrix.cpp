/*
 * This file is part of FeatureHashing
 * Copyright (C) 2014-2015 Wush Wu
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

#include "hashed_model_matrix.h"

using namespace Rcpp;

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

template<typename DataFrameLike>
const ConvertersVec get_converters(
  const NameClassMapping& reference_class, RObject tf, DataFrameLike data, HashFunction* _h_main, HashFunction* _h_binary, size_t hash_size
  ) {
  NumericMatrix tfactors(wrap(tf.attr("factors")));
  CharacterVector reference_name, feature_name;
  Environment feature_hashing(Environment::namespace_env("FeatureHashing"));
  Function parse_special(feature_hashing["parse_special"]);
  std::map<int, std::string> specials;
  {
    List tmp(tf.attr("specials"));
    CharacterVector tmp_name(tmp.attr("names"));
    for(int i = 0;i < tmp.size();i++) {
      SEXP ptag = tmp[i];
      if (!Rf_isNull(ptag)) {
        IntegerVector tmp_index(tmp[i]);
        const char* callback_generator_name = CHAR(wrap(tmp_name[i]));
#ifdef NOISY_DEBUG
        Rprintf("Extract generator: %s from .callback\n", CHAR(wrap(tmp_name[i])));
#endif
        std::for_each(tmp_index.begin(), tmp_index.end(), [&specials, &callback_generator_name](const int index) {
          specials.insert(std::make_pair(index, callback_generator_name));
        });
      }
    }
#ifdef NOISY_DEBUG
    for(auto i = specials.begin();i != specials.end();i++) {
      Rprintf("special %s at index: %d\n", i->second.c_str(), i->first);
    }
#endif
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
        const auto j_special = specials.find(j + 1);
        if (j_special == specials.end()) {
          if (reference_class.find(rname) == reference_class.end()) throw std::invalid_argument("Failed to find the column:");
          const std::string& rclass(reference_class.find(rname)->second);
          #ifdef NOISY_DEBUG
          Rprintf("rclass: %s\n", rclass.c_str());
          #endif
          Param param(rname, _h_main, _h_binary, hash_size);
          if (rclass.compare("factor") == 0) {
            #ifdef NOISY_DEBUG
            Rprintf("Initialize FactorConverter\n");
            #endif
            p.reset(new FactorConverter(wrap(data[rname.c_str()]), param));
          } else if (rclass.compare("numeric") == 0) {
            #ifdef NOISY_DEBUG
            Rprintf("Initialize NumConverter\n");
            #endif
            p.reset(new NumConverter(wrap(data[rname.c_str()]), param));
          } else if (rclass.compare("integer") == 0) {
            #ifdef NOISY_DEBUG
            Rprintf("Initialize IntConverter\n");
            #endif
            p.reset(new IntConverter(wrap(data[rname.c_str()]), param));
          } else if (rclass.compare("logical") == 0) {
            #ifdef NOISY_DEBUG
            Rprintf("Initialize LogicalConverter\n");
            #endif
            p.reset(new LogicalConverter(wrap(data[rname.c_str()]), param));            
          } else if (rclass.compare("character") == 0) {
            #ifdef NOISY_DEBUG
            Rprintf("Initialize CharacterConverter\n");
            #endif
            p.reset(new CharacterConverter(wrap(data[rname.c_str()]), param));
          } else {
            throw std::invalid_argument("Non supported type at name: ");
          }
        } 
        else {
          #ifdef NOISY_DEBUG
          Rprintf(" (parsing spetial..) text: %s special: %s\n", rname.c_str(), j_special->second.c_str());
          #endif
          RObject callback_functor(parse_special(wrap(rname), wrap(j_special->second.c_str()), data));
          rname.assign(as<std::string>(callback_functor.attr("rname")));
          Param param(rname, _h_main, _h_binary, hash_size);
          #ifdef NOISY_DEBUG
          Rprintf(" (rname ==> %s) ", rname.c_str());
          #endif
          if (reference_class.find(rname) == reference_class.end()) {
            throw std::invalid_argument("The first argument of the callback should be one of the column name of the data");
          }
          #ifdef NOISY_DEBUG
          Rprintf("Initialize CallbackConverter\n");
          Rprintf("Test h_main: %zu\n", (*param.h_main)("test", 4));
          Rprintf("Test h_binary: %zu\n", (*param.h_binary)("test", 4));
          #endif
          p.reset(new CallbackConverter(as<CallbackFunctor*>(callback_functor), param));
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
        Param param("", _h_main, _h_binary, hash_size);
        *retval.rbegin() = pInteractionConverter(new InteractionConverter(q, p, param));
      }
    }
  }
  return retval;
} 

template<typename DataFrameLike>
SEXP hashed_model_matrix(RObject tf, DataFrameLike data, unsigned long hash_size, bool transpose, S4 retval, bool keep_hashing_mapping, bool is_xi, bool progress) {
  if (hash_size > 4294967296) throw std::invalid_argument("hash_size is too big!");
  NameClassMapping reference_class(get_class(data));
  Environment e(Environment::base_env().new_child(wrap(true)));
  std::shared_ptr<HashFunction> pHF(NULL), pBHF(NULL);
  if (keep_hashing_mapping) {
    pHF.reset(new MurmurHash3LogHashFunction(wrap(e), MURMURHASH3_H_SEED));
  } else {
    pHF.reset(new MurmurHash3HashFunction(MURMURHASH3_H_SEED));
  }
  if (is_xi) pBHF.reset(new MurmurHash3HashFunction(MURMURHASH3_XI_SEED));
  else pBHF.reset(new NullHashFunction);
  ConvertersVec converters(get_converters(reference_class, tf, data, pHF.get(), pBHF.get(), hash_size));
  #ifdef NOISY_DEBUG
  Rprintf("The size of convertres is %d\n", converters.size());
  #endif
  std::vector<int> ivec, pvec(1, 0);
  std::vector<double> xvec;
  bool is_intercept = as<bool>(tf.attr("intercept"));
  #ifdef NOISY_DEBUG
  Rprintf("nrow(data): %d length(converters): %d\n", data.nrows(), converters.size());
  #endif
  std::shared_ptr<boost::progress_display> pd(NULL);
  if (transpose) {
    if (progress) pd.reset(new boost::progress_display(data.nrows(), Rcpp::Rcout));
    for(auto i = 0;i < data.nrows();i++) {
      if (progress) ++(*pd);
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
          ivec.push_back(hashed_value);
        });
        xvec.insert(xvec.end(), x_origin.begin(), x_origin.end());
      }
      pvec.push_back(ivec.size());
    }
  }
  else {
    if (progress) pd.reset(new boost::progress_display(data.nrows(), Rcpp::Rcout));
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
      if (progress) ++(*pd);
      for(auto j = converters.begin();j != converters.end();j++) {
        pVectorConverter& p(*j);
        const std::vector<uint32_t>& i_origin(p->get_feature(i));
        const std::vector<double>& x_origin(p->get_value(i));
        auto x_value = x_origin.begin();
        std::for_each(i_origin.begin(), i_origin.end(), [&cache, &hash_size, &x_value, &i](uint32_t hashed_value) {
          std::pair< std::vector<int>, std::vector<double> >& k(cache[hashed_value]);
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
  {
    CharacterVector key(e.ls(true));
    std::for_each(key.begin(), key.end(), [&e, &hash_size](const char* s) {
      uint32_t *p = (uint32_t*) INTEGER(e[s]);
      p[0] = p[0] % hash_size;
    });
  }
  retval.attr("mapping") = e;
  return retval;
}

//[[Rcpp::export(".hashed.model.matrix.dataframe")]]
SEXP hashed_model_matrix_dataframe(RObject tf, DataFrame data, unsigned long hash_size, bool transpose, S4 retval, bool keep_hashing_mapping, bool is_xi, bool progress) {
  return hashed_model_matrix<DataFrame>(tf, data, hash_size, transpose, retval, keep_hashing_mapping, is_xi, progress);
}
