#include "invind/indexer.hpp"
#include "invind/sorted_vector.hpp"

namespace invind {
Indexer::Query::Query(Indexer *indexer, const json &query, bool or_scope)
    : query_json(query), indexer(indexer), or_scope(or_scope) {}

SortedVector Indexer::Query::execute() const {
  SortedVector result{true};
  for (const auto &col_index : indexer->categorical_name_to_index_) {
    try {
      auto &query_value_ref = query_json.at(col_index.first);
      if (query_value_ref.is_null()) {
        // nothing to specify
        continue;
      }
      auto query = query_value_ref.get<std::string>();
      if (this->or_scope) {
        result = result.set_or(
            indexer->categorical_fields[col_index.second]->get_match(query));
      } else {
        result = result.set_and(
            indexer->categorical_fields[col_index.second]->get_match(query));
      }

    } catch (json::out_of_range) {
      // no specification. continue;
      continue;
    };
  }
  return result;
}

} // namespace invind