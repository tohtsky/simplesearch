#ifndef INVIND_DATA_HPP
#define INVIND_DATA_HPP

#include "field.hpp"
#include "invind/sorted_vector.hpp"
#include "nlohmann/json.hpp"
#include <bits/stdint-uintn.h>
#include <cstdint>
#include <memory>
#include <unordered_map>

using json = nlohmann::json;
namespace invind {
struct Indexer {

  Indexer();

  Indexer &add_categorical(std::string field_name);
  Indexer &add_many_to_many(std::string field_name);

  std::string add_index(const json &data);
  uint64_t size() const;
  std::vector<uint64_t> query_execute(const json &query);

private:
  std::vector<Categorical> categorical_fields;
  std::vector<ManyToMany> mtom_fields;
  std::unordered_map<std::string, uint64_t> categorical_name_to_index_;
  std::unordered_map<std::string, uint64_t> mtom_name_to_index_;

  std::vector<uint64_t> indices;
  class Query {

  public:
    Query(Indexer *indexer, const json &query, bool or_scope);

    SortedVector execute() const;

    void raise_categorical_error(const std::string &colname) const;
    void raise_mtom_error(const std::string &colname) const;

    const Indexer *indexer;
    const nlohmann::json query_json;
    const bool or_scope;
  };
};
}; // namespace invind

#endif