#include "simplesearch/field.hpp"
#include "simplesearch/sorted_vector.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>

namespace simplesearch {
using sorted_vectors_type = std::vector<const SortedVector *>;

BaseField::BaseField() {}

Categorical::Categorical() : BaseField(), value_to_indices() {}

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

Numeric::Numeric() : value_storage(), nones() {}
void Numeric::add_value(Key value, uint64_t index) {
  value_storage.insert({value, index});
}
void Numeric::add_none(uint64_t index) { nones.push_back(index); }

SortedVector Numeric::get_match(Key value) const {
  using citer = decltype(value_storage.begin());
  citer iter, end;
  std::tie(iter, end) = value_storage.equal_range(value);
  std::vector<uint64_t> indices;
  while (iter != end) {
    indices.push_back(iter->second);
    iter++;
  }
  return SortedVector{indices};
}

SortedVector Numeric::get_range_ge_le(Key le, Key ge) const {
  if (le > ge) {
    return SortedVector{false};
  }
  return get_range_generic(le, ge, false, false);
}
SortedVector Numeric::get_range_ge_lt(Key le, Key gt) const {
  if (le >= gt) {
    return SortedVector{false};
  }
  return get_range_generic(le, gt, false, true);
}

SortedVector Numeric::get_range_gt_le(Key lt, Key ge) const {
  if (lt >= ge) {
    return SortedVector{false};
  }
  return get_range_generic(lt, ge, true, false);
}
SortedVector Numeric::get_range_gt_lt(Key lt, Key gt) const {
  if (lt >= gt) {
    return SortedVector{false};
  }
  return get_range_generic(lt, gt, true, true);
}
SortedVector Numeric::get_range_generic(double l, double u, bool lower_strict,
                                        bool upper_strict) const {
  auto it = value_storage.lower_bound(l);
  auto last = value_storage.upper_bound(u);
  std::vector<uint64_t> indices;
  while (it != last) {
    if (lower_strict && ((it->first) == l)) {
      ++it;
      continue;
    }
    if (upper_strict && ((it->first) == u)) {
      ++it;
      continue;
    }
    indices.push_back(it->second);
    ++it;
  }
  std::sort(indices.begin(), indices.end());
  return {indices};
}

SortedVector Numeric::get_range_ge(Key le) const {
  return get_right_open_range_generic(le, false);
}

SortedVector Numeric::get_range_gt(Key le) const {
  return get_right_open_range_generic(le, true);
}

SortedVector Numeric::get_right_open_range_generic(Key lower_bound,
                                                   bool strict) const {
  auto it = value_storage.lower_bound(lower_bound);
  std::vector<uint64_t> indices;

  while (it != value_storage.end()) {
    if (strict && (it->first == lower_bound)) {
      ++it;
      continue;
    }
    indices.push_back(it->second);
    ++it;
  }
  std::sort(indices.begin(), indices.end());
  return SortedVector{indices};
}

SortedVector Numeric::get_range_le(Key ge) const {
  return get_left_open_range_generic(ge, false);
}

SortedVector Numeric::get_range_lt(Key gt) const {
  return get_left_open_range_generic(gt, true);
}

SortedVector Numeric::get_left_open_range_generic(double u, bool strict) const {
  /*
  should consider 4 cases.
  1) values are empty => return "ALL"
  2) MAX(values) < u: => return "ALL"
  4) u < MIN(values) => return "EMPTY"
  3) MIN(values) <= u
  */
  if (value_storage.empty()) {
    return {false};
  }
  auto max_value = value_storage.rbegin()->first;
  if (u > max_value) {
    return SortedVector{true};
  }
  auto min_value = value_storage.begin()->first;
  if (u < min_value) {
    return SortedVector{false};
  }
  std::vector<uint64_t> indices;
  auto iter = value_storage.begin();
  auto end = value_storage.upper_bound(u);
  while (iter != end) {
    if (strict && (iter->first == u)) {
      ++iter;
      continue;
    }
    indices.push_back(iter->second);
    ++iter;
  }
  std::sort(indices.begin(), indices.end());
  return {indices};
}
SortedVector Numeric::get_none() const { return this->nones; }

} // namespace simplesearch
