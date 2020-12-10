#include "invind/indexer.hpp"
#include "invind/field.hpp"
#include "nlohmann/json.hpp"
#include <cstdint>
#include <sstream>
#include <stdexcept>

namespace invind {
Indexer::Indexer() {}
uint64_t Indexer::size() const { return this->indices.size(); }

std::string Indexer::repr() const {
  std::stringstream ss;
  ss << "Indexer(size=" << this->size() << ", categorical_columns=[";
  int i = 0;
  for (const auto &c : this->categorical_name_to_index_) {
    ss << "\"" << c.first << "\"";
    if (i < (static_cast<int>(this->categorical_name_to_index_.size()) - 1)) {
      ss << ", ";
    }
    i++;
  }
  ss << "], many_to_many_columns=[";

  i = 0;
  for (const auto &c : this->mtom_name_to_index_) {
    ss << "\"" << c.first << "\"";
    if (i < (static_cast<int>(this->mtom_name_to_index_.size()) - 1)) {
      ss << "\", ";
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
  categorical_name_to_index_[field_name] = categorical_name_to_index_.size();
  categorical_fields.emplace_back();
  return *this;
}
Indexer &Indexer::add_many_to_many(std::string field_name) {
  if (!(mtom_name_to_index_.find(field_name) == mtom_name_to_index_.cend())) {
    throw std::invalid_argument("duplicate field name");
  }
  mtom_name_to_index_[field_name] = mtom_name_to_index_.size();
  mtom_fields.emplace_back();
  return *this;
}
std::string Indexer::add_index(const json &data) {
  std::stringstream ss;
  uint64_t index = this->size();
  this->indices.push_back(index);
  for (const auto &c : categorical_name_to_index_) {
    try {
      std::string category = data.at(c.first).get<std::string>();
      categorical_fields[c.second].add_value(category, index);
    } catch (nlohmann::json::out_of_range) {
      categorical_fields[c.second].add_none(index);
    } catch (nlohmann::json::type_error) {
      ss << "At Index [" << index
         << "], found type error for categorical column \"" << c.first << "\","
         << std::endl;
      categorical_fields[c.second].add_none(index);
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

} // namespace invind