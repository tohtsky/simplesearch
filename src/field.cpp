#include "invind/field.hpp"
#include "invind/sorted_vector.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <iostream>

namespace invind {
BaseField::BaseField() {}

Categorical::Categorical() : BaseField(), value_to_indices() {
}

void Categorical::add_value(const std::string &value, uint64_t index) {
  auto it = value_to_indices.find(value);
  if (it != value_to_indices.end()) {
    auto field_index = it->second;
    this->indices[field_index].push_back(index);

  } else {
    uint64_t new_index = this->value_to_indices.size();
    this->value_to_indices.insert({value, new_index});
    this->indices.emplace_back();
    this->indices.back().push_back(index);
  }
}
void Categorical::add_none(uint64_t index) { this->nones.push_back(index); }

SortedVector Categorical::get_match(const std::string &value) const {
  try {
    auto index = this->value_to_indices.at(value);
    return this->indices[index];
  } catch (std::out_of_range &_) {
    SortedVector empty;
    return empty;
  }
  throw std::runtime_error("something nasty happend for get_match.");
}
SortedVector Categorical::get_none() const { return this->nones; }

ManyToMany::ManyToMany() {}
SortedVector
ManyToMany::include_all(const std::vector<std::string> &values) const {
  if (values.empty()) {
    return SortedVector{true};
  }
  SortedVector result = this->get_match(values[0]);
  for (uint64_t i = 1; i < values.size(); i++) {
    result = result.set_and(this->get_match(values[i]));
  }
  return result;
}

SortedVector
Categorical::include_one(const std::vector<std::string> &values) const {
  if (values.empty()) {
    return SortedVector{};
  }
  SortedVector result(false);
  for (uint64_t i = 0; i < values.size(); i++) {
    result = result.set_or(this->get_match(values[i]));
  }
  return result;
}
} // namespace invind
