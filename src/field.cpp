#include "simplesearch/field.hpp"
#include "simplesearch/sorted_vector.hpp"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>

namespace simplesearch {
using sorted_vectors_type = std::vector<const SortedVector *>;
inline SortedVector
_mergeMultipleSortedVectors(const std::vector<const SortedVector *> &vs) {
  if (vs.size() == 0) {
    return SortedVector{false};
  } else if (vs.size() == 1) {
    SortedVector result(*vs[0]);
    return result;
  } else if (vs.size() == 2) {
    return vs[0]->set_or(*vs[1]);
  }
  std::vector<uint64_t> result;
  for (auto ptr : vs) {
    std::copy(ptr->cbegin(), ptr->cend(), std::back_inserter(result));
  }
  std::sort(result.begin(), result.end());
  return SortedVector{std::move(result)};
}
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

Numeric::Numeric() : value_to_indices(), nones() {}
void Numeric::add_value(Key value, uint64_t index) {
  auto iterator = value_to_indices.find(value);
  if (iterator == value_to_indices.end()) {
    SortedVector index_vector;
    index_vector.push_back(index);
    value_to_indices.insert({value, index_vector});
  } else {
    iterator->second.push_back(index);
  }
}
void Numeric::add_none(uint64_t index) { nones.push_back(index); }
SortedVector Numeric::get_match(Key value) const {
  auto iterator = value_to_indices.find(value);
  if (iterator == value_to_indices.cend()) {
    return SortedVector{};
  }
  return iterator->second;
}

SortedVector Numeric::get_range_ge_le(Key le, Key ge) const {
  if (le > ge) {

    SortedVector result;
  }
  auto it = value_to_indices.lower_bound(le);
  auto last = value_to_indices.upper_bound(ge);
  sorted_vectors_type svs;
  while (it != last) {
    svs.push_back(&(it->second));
    ++it;
  }
  return _mergeMultipleSortedVectors(svs);
}
SortedVector Numeric::get_range_ge_lt(Key le, Key gt) const {
  if (le >= gt) {
    SortedVector result;
    return result;
  }
  auto it = value_to_indices.lower_bound(le);
  auto last = value_to_indices.upper_bound(gt);
  sorted_vectors_type svs;
  while (it != last) {
    if (it->first < gt) {
      svs.push_back(&(it->second));
    }
    ++it;
  }
  return _mergeMultipleSortedVectors(svs);
}
SortedVector Numeric::get_range_gt_le(Key lt, Key ge) const {
  if (lt >= ge) {

    SortedVector result;
    return result;
  }
  auto it = value_to_indices.lower_bound(lt);
  auto last = value_to_indices.upper_bound(ge);
  sorted_vectors_type svs;
  while (it != last) {
    if (it->first > lt) {
      svs.push_back(&(it->second));
    }
    ++it;
  }
  return _mergeMultipleSortedVectors(svs);
}
SortedVector Numeric::get_range_gt_lt(Key lt, Key gt) const {
  if (lt >= gt) {
    SortedVector result;
    return result;
  }
  auto it = value_to_indices.lower_bound(lt);
  auto last = value_to_indices.upper_bound(gt);
  sorted_vectors_type svs;
  while (it != last) {
    if ((it->first > lt) && (it->first < gt)) {
      svs.push_back(&(it->second));
    }
    ++it;
  }
  return _mergeMultipleSortedVectors(svs);
}

SortedVector Numeric::get_range_ge(Key le) const {
  auto it = value_to_indices.lower_bound(le);
  sorted_vectors_type svs;
  while (it != value_to_indices.cend()) {
    svs.push_back(&(it->second));
    ++it;
  }
  return _mergeMultipleSortedVectors(svs);
}
SortedVector Numeric::get_range_gt(Key lt) const {
  auto it = value_to_indices.lower_bound(lt);
  sorted_vectors_type svs;

  while (it != value_to_indices.cend()) {
    if (it->first > lt) {
      svs.push_back(&(it->second));
    }
    ++it;
  }
  return _mergeMultipleSortedVectors(svs);
}
SortedVector Numeric::get_range_le(Key ge) const {
  if (value_to_indices.lower_bound(ge) == value_to_indices.cend()) {
    // for all value, x >= ge
    SortedVector result;
    return result;
  }
  auto iter = value_to_indices.cbegin();
  auto last = value_to_indices.upper_bound(ge);
  sorted_vectors_type svs;
  while (iter != last) {
    svs.push_back(&(iter->second));
    ++iter;
  }
  return _mergeMultipleSortedVectors(svs);
}

SortedVector Numeric::get_range_lt(Key ge) const {
  if (value_to_indices.lower_bound(ge) == value_to_indices.cend()) {
    // for all value, x >= ge
    // x <= ge

    SortedVector result;
    return result;
  }
  auto iter = value_to_indices.cbegin();
  auto last = value_to_indices.upper_bound(ge);
  sorted_vectors_type svs;
  while (iter != last) {
    if (iter->first < ge) {
      svs.push_back(&(iter->second));
    }
    ++iter;
  }
  return _mergeMultipleSortedVectors(svs);
}
SortedVector Numeric::get_none() const { return this->nones; }

} // namespace simplesearch
