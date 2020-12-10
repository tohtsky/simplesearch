#include "catch.hpp"
#include "invind/sorted_vector.hpp"
#include <map>
#include <set>
using namespace invind;

using Key = double;
struct NumericField {
  NumericField();
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
  inline MapType::const_iterator le_iterator(Key v) {
    return value_to_indices.lower_bound(v);
  }
  inline MapType::const_iterator lt_iterator(Key v) {
    auto cand_iter = value_to_indices.lower_bound(v);
    if (cand_iter == value_to_indices.cend()) {
      return cand_iter;
    } else {
      if (cand_iter->first == v) {
        cand_iter++;
        return cand_iter;
      } else {
        return cand_iter;
      }
    }
  }

  SortedVector nones;
};
NumericField::NumericField() : value_to_indices(), nones() {}
void NumericField::add_value(Key value, uint64_t index) {
  auto iterator = value_to_indices.find(value);
  if (iterator == value_to_indices.end()) {
    SortedVector index_vector;
    index_vector.push_back(index);
    value_to_indices.insert({value, index_vector});
  } else {
    iterator->second.push_back(index);
  }
}
void NumericField::add_none(uint64_t index) { nones.push_back(index); }
SortedVector NumericField::get_match(Key value) const {
  auto iterator = value_to_indices.find(value);
  if (iterator == value_to_indices.cend()) {
    return SortedVector{};
  }
  return iterator->second;
}

SortedVector NumericField::get_range_ge_le(Key le, Key ge) const {
  if (le > ge) {
    throw std::invalid_argument("le > ge holds.");
  }
  SortedVector result;
  auto it = value_to_indices.lower_bound(le);
  auto last = value_to_indices.upper_bound(ge);
  while (it != last) {
    result = result.set_or(it->second);
    ++it;
  }
  return result;
}
SortedVector NumericField::get_range_ge_lt(Key le, Key gt) const {
  if (le >= gt) {
    throw std::invalid_argument("le >= gt holds.");
  }
  SortedVector result;
  auto it = value_to_indices.lower_bound(le);
  auto last = value_to_indices.upper_bound(gt);
  while (it != last) {
    if (it->first < gt) {
      result = result.set_or(it->second);
    }
    ++it;
  }
  return result;
}
SortedVector NumericField::get_range_gt_le(Key lt, Key ge) const {
  if (lt >= ge) {
    throw std::invalid_argument("lt >= ge holds.");
  }
  SortedVector result;
  auto it = value_to_indices.lower_bound(lt);
  auto last = value_to_indices.upper_bound(ge);
  while (it != last) {
    if (it->first > lt) {
      result = result.set_or(it->second);
    }
    ++it;
  }
  return result;
}
SortedVector NumericField::get_range_gt_lt(Key lt, Key gt) const {
  if (lt >= gt) {
    throw std::invalid_argument("lt >= gt holds.");
  }
  SortedVector result;
  auto it = value_to_indices.lower_bound(lt);
  auto last = value_to_indices.upper_bound(gt);
  while (it != last) {
    if ((it->first > lt) && (it->first < gt)) {
      result = result.set_or(it->second);
    }
    ++it;
  }
  return result;
}

SortedVector NumericField::get_range_ge(Key le) const {
  SortedVector result;
  auto it = value_to_indices.lower_bound(le);
  while (it != value_to_indices.cend()) {
    result = result.set_or(it->second);
    ++it;
  }
  return result;
}
SortedVector NumericField::get_range_gt(Key lt) const {
  SortedVector result;
  auto it = value_to_indices.lower_bound(lt);
  while (it != value_to_indices.cend()) {
    if (it->first > lt) {
      result = result.set_or(it->second);
    }
    ++it;
  }
  return result;
}
SortedVector NumericField::get_range_le(Key ge) const {
  SortedVector result;
  if (value_to_indices.lower_bound(ge) == value_to_indices.cend()) {
    // every element x >= ge
    // for all value, x >= ge
    // x <= ge
    return result;
  }
  auto iter = value_to_indices.cbegin();
  auto last = value_to_indices.upper_bound(ge);
  while (true) {
    result = result.set_or(iter->second);
    ++iter;
  }
  return result;
}

SortedVector NumericField::get_range_lt(Key ge) const {
  SortedVector result;
  if (value_to_indices.lower_bound(ge) == value_to_indices.cend()) {
    // for all value, x >= ge
    // x <= ge
    return result;
  }
  auto iter = value_to_indices.cbegin();
  auto last = value_to_indices.upper_bound(ge);
  while (true) {
    if (iter->first < ge) {
      result = result.set_or(iter->second);
    }
    ++iter;
  }
  return result;
}

TEST_CASE("btee", "[usage]") {
  NumericField field;
  auto sv_to_v = [](const SortedVector &sv) {
    std::vector<uint64_t> result;
    for (auto c = sv.cbegin(); c != sv.cend(); ++c) {
      result.push_back(*c);
    }
    return result;
  };
  std::vector<double> values{1.0, 2.0, 1.00001, 1.999};
  for (uint64_t i = 0; i < values.size(); i++) {
    field.add_value(values[i], i);
  }
  {
    auto result = sv_to_v(field.get_match(1.0));
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 0);
  }
  {
    auto result = sv_to_v(field.get_range_gt_le(1.0, 2.0));
    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == 1);
    REQUIRE(result[1] == 2);
    REQUIRE(result[2] == 3);
  }
  {
    auto result = sv_to_v(field.get_range_ge_lt(1.0, 2.0));
    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == 0);
    REQUIRE(result[1] == 2);
    REQUIRE(result[2] == 3);
  }
  {
    auto result = sv_to_v(field.get_range_ge_lt(1.0, 2.0));
    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == 0);
    REQUIRE(result[1] == 2);
    REQUIRE(result[2] == 3);
  }
  {
    auto result = sv_to_v(field.get_range_gt_lt(1.0, 2.0));
    REQUIRE(result.size() == 2);
    REQUIRE(result[0] == 2);
    REQUIRE(result[1] == 3);
  }
  {
    auto result = sv_to_v(field.get_range_gt_lt(1.0, 2.0));
    REQUIRE(result.size() == 2);
    REQUIRE(result[0] == 2);
    REQUIRE(result[1] == 3);
  }

  /* somewhat smaller upper bounds */
  {
    auto result = sv_to_v(field.get_range_gt_le(-1.00, 0.99));
    REQUIRE(result.size() == 0);
  }
  {
    auto result = sv_to_v(field.get_range_gt_le(-1.00, 1.00));
    REQUIRE(result.size() == 1);
  }
  {
    auto result = sv_to_v(field.get_range_gt_lt(-1.00, 1.00));
    REQUIRE(result.size() == 0);
  }

  /* somewhat larger lower bounds */
  {
    auto result = sv_to_v(field.get_range_gt_le(2.00, 10000));
    REQUIRE(result.size() == 0);
  }
  {
    auto result = sv_to_v(field.get_range_ge_lt(2.00, 100000));
    REQUIRE(result.size() == 1);
  }
  {
    auto result = sv_to_v(field.get_range_gt_lt(2.01, 100000));
    REQUIRE(result.size() == 0);
  }
}