#include "invind/indexer.hpp"
#include "invind/sorted_vector.hpp"
#include <sstream>
#include <stdexcept>

namespace invind {
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

  return result;
}

} // namespace invind