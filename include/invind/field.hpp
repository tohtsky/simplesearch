#ifndef INVIND_FIELD_HPP
#define INVIND_FIELD_HPP

#include "sorted_vector.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace invind {
struct BaseField {
  BaseField();
  virtual uint64_t size() const;

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

} // namespace invind
#endif