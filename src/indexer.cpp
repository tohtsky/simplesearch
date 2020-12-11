#include "simplesearch/indexer.hpp"
#include "simplesearch/field.hpp"
#include "nlohmann/json.hpp"
#include <cstdint>
#include <sstream>
#include <stdexcept>

namespace simplesearch {
Indexer::Indexer() {}
uint64_t Indexer::size() const { return this->indices.size(); }

std::string Indexer::repr() const {
  std::stringstream ss;
  ss << "Indexer(size=" << this->size() << ", categorical_columns=[";
  int i = 0;
  for (const auto &c : this->categorical_name_to_index_) {
    ss << "\"" << c.first << "\"";
    if (i < (static_cast<int>(this->categorical_fields.size()) - 1)) {
      ss << ", ";
    }
    i++;
  }
  ss << "], many_to_many_columns=[";

  i = 0;
  for (const auto &c : this->mtom_name_to_index_) {
    ss << "\"" << c.first << "\"";
    if (i < (static_cast<int>(this->mtom_fields.size()) - 1)) {
      ss << ", ";
    }
    i++;
  }
  ss << "], numeric_columns=[";
  i = 0;
  for (const auto &c : this->numeric_name_to_index_) {
    ss << "\"" << c.first << "\"";
    if (i < (static_cast<int>(this->num_fields.size()) - 1)) {
      ss << ", ";
    }
    i++;
  }
  ss << "]";
  ss << ")";
  return ss.str();
}

Indexer &Indexer::add_categorical(std::string field_name) {
  if (!(categorical_name_to_index_.find(field_name) ==
        categorical_name_to_index_.cend())) {
    throw std::invalid_argument("duplicate field name");
  }
  categorical_name_to_index_.insert({field_name, categorical_fields.size()});
  categorical_fields.emplace_back();
  return *this;
}

Indexer &Indexer::add_many_to_many(std::string field_name) {
  if (!(mtom_name_to_index_.find(field_name) == mtom_name_to_index_.cend())) {
    throw std::invalid_argument("duplicate field name");
  }
  mtom_name_to_index_.insert({field_name, mtom_fields.size()});
  mtom_fields.emplace_back();
  return *this;
}

Indexer &Indexer::add_numerical(std::string field_name) {
  if (!(numeric_name_to_index_.find(field_name) ==
        numeric_name_to_index_.cend())) {
    throw std::invalid_argument("duplicate field name");
  }
  numeric_name_to_index_.insert({field_name, num_fields.size()});
  num_fields.emplace_back();
  return *this;
}

std::string Indexer::add_index(const json &data) {
  std::stringstream ss;
  uint64_t index = this->size();
  this->indices.push_back(index);
  for (auto iter = categorical_name_to_index_.begin();
       iter != categorical_name_to_index_.end(); iter++) {
    try {
      std::string category = data.at(iter->first).get<std::string>();
      categorical_fields[iter->second].add_value(category, index);
    } catch (nlohmann::json::out_of_range) {
      categorical_fields[iter->second].add_none(index);
    } catch (nlohmann::json::type_error) {
      ss << "At Index [" << index
         << "], found type error for categorical column \"" << iter->first
         << "\"," << std::endl;
      categorical_fields[iter->second].add_none(index);
    }
  }
  for (const auto &c : mtom_name_to_index_) {
    std::string keyname = c.first;
    try {
      auto &d = data.at(keyname);
      auto values = d.get<std::vector<std::string>>();
      for (auto &v : values) {
        mtom_fields[c.second].add_value(v, index);
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

  for (const auto &c : numeric_name_to_index_) {
    std::string keyname = c.first;
    try {
      auto &d = data.at(keyname);
      if (d.is_null()) {
        num_fields[c.second].add_none(index);
      } else {
        auto value = d.get<double>();
        num_fields[c.second].add_value(value, index);
      }
    } catch (nlohmann::json::out_of_range) {
      continue;
    } catch (nlohmann::json::type_error) {
      ss << "At Index [" << index << "], found type error for numeric column \""
         << c.first << "\"," << std::endl;
      continue;
    }
  }
  return ss.str();
}

std::vector<uint64_t> Indexer::query_execute(const json &query) const {
  Query query_obj(this, query, false);
  auto result = query_obj.execute();
  if (result.is_all()) {
    std::vector<uint64_t> rv(this->indices);
    return rv;
  } else {
    return result.move_result();
  }
}

} // namespace simplesearch