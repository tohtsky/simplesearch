#include "simplesearch/indexer.hpp"
#include "simplesearch/sorted_vector.hpp"
#include <sstream>
#include <stdexcept>

namespace simplesearch {
Indexer::Query::Query(const Indexer *indexer, const json &query, bool or_scope)
    : query_json(query), indexer(indexer), or_scope(or_scope) {}

void Indexer::Query::raise_categorical_error(const std::string &colname) const {
  std::stringstream ss;
  ss << R"(categorical field ")" << colname << R"(" requires:
    : string,
    : vector<string>,
    : null (no condition)
)";
  throw std::invalid_argument(ss.str());
}
void Indexer::Query::raise_mtom_error(const std::string &colname) const {
  std::stringstream ss;

  ss << R"(Many to many field ")" << colname << R"(" requires
  : List[string], (equivalent to {"contains_one": [...]})
  : {"contains_one": List[str]}
  : {"contains_all": List[str]}
  : null (no condition)
  )";
  throw std::invalid_argument(ss.str());
}

void Indexer::Query::raise_numeric_error(const std::string &colname) const {
  std::stringstream ss;

  ss << R"(numerical field ")" << colname << R"(" requires
  : value(double), (equivalent to {"match": value})
  : {"match": value}
  : {"le": value, "lt": value "ge": value, "gt": value} (at least one key must be present)
  : null (no condition)
  )";
  throw std::invalid_argument(ss.str());
}

SortedVector Indexer::Query::execute() const {
  SortedVector result{true};
  // categorical
  for (const auto &col_index : indexer->categorical_name_to_index_) {
    try {
      auto &query_value_ref = query_json.at(col_index.first);
      if (query_value_ref.is_null()) {
        // nothing to specify
        continue;
      }
      if (query_value_ref.is_string()) {
        auto query = query_value_ref.get<std::string>();
        if (this->or_scope) {
          result = result.set_or(
              indexer->categorical_fields[col_index.second].get_match(query));
        } else {
          result = result.set_and(
              indexer->categorical_fields[col_index.second].get_match(query));
        }
      } else if (query_value_ref.is_array()) {
        auto query_vals = query_value_ref.get<std::vector<std::string>>();
        SortedVector subresult =
            indexer->categorical_fields[col_index.second].include_one(
                query_vals);
        if (this->or_scope) {
          result = result.set_or(subresult);
        } else {
          result = result.set_and(subresult);
        }
      }
    } catch (json::out_of_range) {
      // no specification. continue;
    } catch (json::type_error) {
      this->raise_categorical_error(col_index.first);
    }
  }

  // multi
  for (const auto &col_index : indexer->mtom_name_to_index_) {
    try {
      auto &query_value_ref = query_json.at(col_index.first);
      if (query_value_ref.is_null()) {
        // nothing to specify
        continue;
      }
      if (query_value_ref.is_primitive()) {
        raise_mtom_error(col_index.first);
      }
      bool contains_one = true;
      std::vector<std::string> codes;
      if (query_value_ref.is_array()) {
        codes = query_value_ref.get<std::vector<std::string>>();
      } else if (query_value_ref.find("contains_one") !=
                 query_value_ref.end()) {
        if (query_value_ref.find("contains_all") != query_value_ref.end()) {
          raise_mtom_error(col_index.first);
        }
        codes =
            query_value_ref.at("contains_one").get<std::vector<std::string>>();
      } else if (query_value_ref.find("contains_all") !=
                 query_value_ref.end()) {
        contains_one = false;
        codes =
            query_value_ref.at("contains_all").get<std::vector<std::string>>();
      } else {
        raise_mtom_error(col_index.first);
      }
      auto &f = indexer->mtom_fields[col_index.second];
      SortedVector subresult;
      if (contains_one) {
        subresult = f.include_one(codes);
      } else {
        subresult = f.include_all(codes);
      }

      if (this->or_scope) {
        result = result.set_or(subresult);
      } else {
        result = result.set_and(subresult);
      }

    } catch (json::out_of_range) {
      continue;
    } catch (json::type_error) {
      raise_mtom_error(col_index.first);
    };
  }

  // numeric
  for (const auto &col_index : indexer->numeric_name_to_index_) {
    const Numeric &field = indexer->num_fields[col_index.second];
    try {
      auto &query_value_ref = query_json.at(col_index.first);
      if (query_value_ref.is_number()) {
        auto subresult = field.get_match(query_value_ref.get<double>());
        if (this->or_scope) {
          result = result.set_or(subresult);
        } else {
          result = result.set_and(subresult);
        }
      } else if (query_value_ref.is_null()) {
        auto subresult = field.get_none();
        if (this->or_scope) {
          result = result.set_or(subresult);
        } else {
          result = result.set_and(subresult);
        }
      } else if (query_value_ref.is_object()) {
        auto end = query_value_ref.cend();
        bool found_match = false, found_gte = false, found_gt = false,
             found_lte = false, found_lt = false;
        double match, gte, gt, lte, lt;
        SortedVector subresult;
        {
          auto iter = query_value_ref.find("match");
          if (iter != end) {
            found_match = true;
            match = iter->get<double>();
          }
        }
        {
          auto iter = query_value_ref.find("gte");
          if (iter != end) {
            found_gte = true;
            gte = iter->get<double>();
          }
        }
        {
          auto iter = query_value_ref.find("gt");
          if (iter != end) {
            found_gt = true;
            gt = iter->get<double>();
          }
        }
        {
          auto iter = query_value_ref.find("lte");
          if (iter != end) {
            found_lte = true;
            lte = iter->get<double>();
          }
        }
        {
          auto iter = query_value_ref.find("lt");
          if (iter != end) {
            found_lt = true;
            lt = iter->get<double>();
          }
        }
        if (found_match) {
          if (found_gte || found_gt || found_lte || found_lt) {
            raise_numeric_error(col_index.first);
          }
          subresult = field.get_match(match);
        } else {
          if (!(found_gte || found_gt || found_lte || found_lt)) {
            raise_numeric_error(col_index.first);
          }

          bool has_lower_bound = false, has_upper_bound = false;
          bool lower_strict = false, upper_strict = false;
          double lower_bound, upper_bound;
          if (found_gte || found_gt) {
            has_lower_bound = true;
            if (found_gte && found_gt) {
              if (gte <= gt) {
                lower_bound = gt;
                lower_strict = true;
              } else {
                lower_bound = gte;
                lower_strict = false;
              }
            } else if (found_gte && !found_gt) {
              lower_bound = gte;
              lower_strict = false;
            } else {
              lower_bound = gt;
              lower_strict = true;
            }
          } else {
            has_lower_bound = false;
          }

          if (found_lte || found_lt) {
            has_upper_bound = true;
            if (found_lte && found_lt) {
              if (lt <= lte) {
                upper_bound = lt;
                upper_strict = true;
              } else {
                upper_bound = lte;
                upper_strict = false;
              }
            } else if (found_lte && !found_lt) {
              upper_bound = lte;
              upper_strict = false;
            } else {
              upper_bound = lt;
              upper_strict = true;
            }
          } else {
            has_upper_bound = false;
          }
          if (has_lower_bound && has_upper_bound) {
            if (lower_strict && upper_strict) {
              subresult = field.get_range_gt_lt(lower_bound, upper_bound);
            } else if (lower_strict && !upper_strict) {
              subresult = field.get_range_gt_le(lower_bound, upper_bound);
            } else if (!lower_strict && upper_strict) {
              subresult = field.get_range_ge_lt(lower_bound, upper_bound);
            } else {
              subresult = field.get_range_ge_le(lower_bound, upper_bound);
            }
          } else if (has_lower_bound && !has_upper_bound) {
            if (lower_strict) {
              subresult = field.get_range_gt(lower_bound);
            } else {
              subresult = field.get_range_ge(lower_bound);
            }
          } else if (!has_lower_bound && has_upper_bound) {
            if (upper_strict) {
              subresult = field.get_range_lt(upper_bound);
            } else {
              subresult = field.get_range_le(upper_bound);
            }
          }
        }
        if (this->or_scope) {
          result = result.set_or(subresult);
        } else {
          result = result.set_and(subresult);
        }
      } else {
        raise_numeric_error(col_index.first);
      }
    } catch (json::out_of_range) {
      continue;
    } catch (json::type_error) {
      raise_mtom_error(col_index.first);
    };
  }

  return result;
}

} // namespace simplesearch