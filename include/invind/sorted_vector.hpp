#ifndef INVIND_SORTED_VECTOR_HPP
#define INVIND_SORTED_VECTOR_HPP

#include <algorithm>
#include <bits/stdint-uintn.h>
#include <iterator>
#include <stdexcept>
#include <vector>

namespace invind {

class SortedVector {
  using IndexType = std::uint64_t;
  using VectorType = std::vector<IndexType>;
  using self_type = SortedVector;

public:
  inline SortedVector(const VectorType &v) : buffer_(v), all_true(false) {
    if (buffer_.empty()) {
      return;
    }
    auto value = buffer_.at(0);
    for (uint64_t i = 1; i < buffer_.size(); i++) {
      auto new_value = buffer_.at(i);
      if (new_value < value) {
        throw std::runtime_error("not sorted");
      }
      value = new_value;
    }
  }
  inline SortedVector() : buffer_(), all_true(false) {}
  inline SortedVector(bool all_true) : buffer_(), all_true(all_true) {}
  inline SortedVector(const self_type &other)
      : buffer_(other.buffer_), all_true(other.all_true) {}
  inline uint64_t size() { return buffer_.size(); }
  using const_iterator = VectorType::const_iterator;

  inline const_iterator cbegin() const { return buffer_.cbegin(); }
  inline const_iterator cend() const { return buffer_.cend(); }

  inline void push_back(const IndexType &val) {
    if (!buffer_.empty()) {
      auto back_element = buffer_.back();
      if (val < back_element) {
        throw std::invalid_argument(
            "push back operation will destroy the sort ordering.");
      }
    }
    buffer_.push_back(val);
  }

  inline void reserve(const uint64_t cap) { buffer_.reserve(cap); }

  inline self_type set_and(const self_type &other) {
    if (this->all_true) {
      return other;
    } else if (other.all_true) {
      return SortedVector{*this};
    }
    self_type result;
    std::set_intersection(this->buffer_.cbegin(), this->buffer_.cend(),
                          other.buffer_.cbegin(), other.buffer_.cend(),
                          std::inserter(result.buffer_, result.buffer_.end()));
    return result;
  }

  inline self_type set_or(const self_type &other) {
    if (this->all_true || other.all_true) {
      return SortedVector{true};
    }
    self_type result;
    std::set_union(this->buffer_.cbegin(), this->buffer_.cend(),
                   other.buffer_.cbegin(), other.buffer_.cend(),
                   std::inserter(result.buffer_, result.buffer_.end()));
    return result;
  }

  inline bool is_all() const { return all_true; }
  inline std::vector<IndexType> &&move_result() {
    this->all_true = false;
    return std::move(this->buffer_);
  }

private:
  std::vector<IndexType> buffer_;
  bool all_true;
};
} // namespace invind

#endif