#include "invind/indexer.hpp"
#include "invind/field.hpp"
#include "nlohmann/json.hpp"
#include <bits/stdint-uintn.h>
#include <cstdint>
#include <sstream>
#include <stdexcept>

namespace invind {
Indexer::Indexer() {}
uint64_t Indexer::size() const { return this->indices.size(); }

Indexer &Indexer::add_categorical(std::string field_name) {
  if (!(categorical_name_to_index_.find(field_name) ==
        categorical_name_to_index_.cend())) {
    throw std::invalid_argument("duplicate field name");
  }
  categorical_name_to_index_[field_name] = categorical_name_to_index_.size();
  auto ptr = new Categorical();
  categorical_fields.emplace_back(ptr);
  return *this;
}
Indexer &Indexer::add_many_to_many(std::string field_name) {
  if (!(mtom_name_to_index_.find(field_name) == mtom_name_to_index_.cend())) {
    throw std::invalid_argument("duplicate field name");
  }
  mtom_name_to_index_[field_name] = mtom_name_to_index_.size();
  auto ptr = new ManyToMany();
  mtom_fields.emplace_back(ptr);
  return *this;
}
std::string Indexer::add_index(const json &data) {
  std::stringstream ss;
  uint64_t index = this->size();
  this->indices.push_back(index);
  for (const auto &c : categorical_name_to_index_) {
    try {
      std::string category = data.at(c.first).get<std::string>();
      categorical_fields[c.second]->add_value(category, index);
    } catch (nlohmann::json::out_of_range) {
      categorical_fields[c.second]->add_none(index);
    } catch (nlohmann::json::type_error) {
      ss << "At Index [" << index
         << "], found type error for categorical column \"" << c.first << "\","
         << std::endl;
      categorical_fields[c.second]->add_none(index);
    }
  }
  for (const auto &c : mtom_name_to_index_) {
    std::string keyname = c.first;
    try {
      auto &d = data.at(keyname);
      auto values = d.get<std::vector<std::string>>();
      for (auto &v : values) {
        mtom_fields[c.second]->add_value(v, index);
      }
    } catch (nlohmann::json::out_of_range) {
      continue;
    } catch (nlohmann::json::type_error) {
      ss << "At Index [" << index
         << "], found type error for many_to_many column \"" << c.first << "\","
         << std::endl;
      continue;
    }
  }
  return ss.str();
}

std::vector<uint64_t> Indexer::query_execute(const json &query) {
  Query query_obj(this, query, false);
  auto result = query_obj.execute();
  if (result.is_all()) {
    std::vector<uint64_t> rv(this->indices);
    return rv;
  } else {
    return result.move_result();
  }
}

} // namespace invind