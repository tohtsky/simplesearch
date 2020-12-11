#include <stdexcept>
#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
                          // in one cpp file
#include "catch.hpp"
#include "simplesearch/sorted_vector.hpp"

#include <iostream>

using namespace simplesearch;
TEST_CASE("basin usage", "[sorted]") {
  // setting up
  SortedVector v1;
  v1.push_back(0);
  v1.push_back(1);
  v1.push_back(3);
  SECTION("basic functionality") { REQUIRE(v1.size() == 3); }
  REQUIRE_THROWS_AS([&]() { v1.push_back(2); }(), std::invalid_argument);

  // setting up v2
  SortedVector v2(std::vector<uint64_t>{1, 2});
  auto v3 = v1.set_or(v2); // should be 0, 1, 2, 3
  REQUIRE(v3.size() == 4);

  auto v4 = v1.set_and(v2); // should be 1 only
  REQUIRE(v4.size() == 1);
}