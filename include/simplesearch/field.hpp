#ifndef INVIND_FIELD_HPP
#define INVIND_FIELD_HPP

#include "sorted_vector.hpp"
#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace simplesearch {
struct BaseField {
  BaseField();

protected:
  std::vector<SortedVector> indices;
};

struct Categorical : BaseField {
  Categorical();
  void add_value(const std::string &, uint64_t index);
  void add_none(uint64_t index);

  SortedVector get_match(const std::string &) const;
  SortedVector get_none() const;
  SortedVector include_one(const std::vector<std::string> &) const;

protected:
  std::unordered_map<std::string, uint64_t> value_to_indices;
  SortedVector nones;
};

struct ManyToMany : Categorical {
  ManyToMany();
  SortedVector include_all(const std::vector<std::string> &) const;
};

using Key = double;
struct Numeric {
  Numeric();
  void add_value(Key, uint64_t);
  void add_none(uint64_t);

  SortedVector get_match(Key) const;

  SortedVector get_range_ge(Key) const;
  SortedVector get_range_gt(Key) const;
  SortedVector get_range_le(Key) const;
  SortedVector get_range_lt(Key) const;

  SortedVector get_range_ge_le(Key, Key) const;
  SortedVector get_range_ge_lt(Key, Key) const;
  SortedVector get_range_gt_le(Key, Key) const;
  SortedVector get_range_gt_lt(Key, Key) const;

  SortedVector get_none() const;

protected:
  using MapType = std::map<Key, SortedVector>;
  MapType value_to_indices;

  SortedVector nones;
};

} // namespace simplesearch

#endif