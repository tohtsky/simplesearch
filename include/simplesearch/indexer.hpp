#ifndef INVIND_DATA_HPP
#define INVIND_DATA_HPP

#include "field.hpp"
#include "simplesearch/sorted_vector.hpp"
#include "nlohmann/json.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

using json = nlohmann::json;
namespace simplesearch {
struct Indexer {

  Indexer();

  Indexer &add_categorical(std::string field_name);
  Indexer &add_many_to_many(std::string field_name);
  Indexer &add_numerical(std::string field_name);

  std::string add_index(const json &data);
  uint64_t size() const;
  std::vector<uint64_t> query_execute(const json &query) const;
  std::string repr() const;

private:
  std::vector<Categorical> categorical_fields;
  std::vector<ManyToMany> mtom_fields;
  std::vector<Numeric> num_fields;
  std::unordered_map<std::string, uint64_t> categorical_name_to_index_;
  std::unordered_map<std::string, uint64_t> mtom_name_to_index_;
  std::unordered_map<std::string, uint64_t> numeric_name_to_index_;

  std::vector<uint64_t> indices;
  class Query {

  public:
    Query(const Indexer *indexer, const json &query, bool or_scope);

    SortedVector execute() const;

    void raise_categorical_error(const std::string &colname) const;
    void raise_mtom_error(const std::string &colname) const;
    void raise_numeric_error(const std::string &colname) const;

    const Indexer *indexer;
    const nlohmann::json query_json;
    const bool or_scope;
  };
};
}; // namespace simplesearch

#endif