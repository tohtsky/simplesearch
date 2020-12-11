#include "catch.hpp"
#include "simplesearch/field.hpp"
#include "simplesearch/indexer.hpp"

using namespace simplesearch;

TEST_CASE("btee", "[usage]") {
  Indexer worker;
  worker.add_numerical("nvalue");
  std::vector<double> values{1.0, 2.0, 1.00001, 1.999};
  for (uint64_t i = 0; i < values.size(); i++) {
    json j;
    j["nvalue"] = values[i];
    worker.add_index(j);
  }
  {
    auto result = worker.query_execute(R"({"nvalue": 1.0})"_json);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 0);
  }
  {
    auto result = worker.query_execute(R"({"nvalue": {"match": 1.05}})"_json);
    REQUIRE(result.size() == 0);
  }
  {
    auto result = worker.query_execute(
        R"({"nvalue":{"gte":1.0, "gt": 1.0, "lte": 2.0, "lt": 2.00001}})"_json);
    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == 1);
    REQUIRE(result[1] == 2);
    REQUIRE(result[2] == 3);
  }
  {
    auto result = worker.query_execute(
        R"({"nvalue":{"gte": 1.0, "lt": 2.0, "lte":2.0}})"_json);
    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == 0);
    REQUIRE(result[1] == 2);
    REQUIRE(result[2] == 3);
  }
  {
    auto result = worker.query_execute(
        R"({"nvalue": {"gte": 1.0, "gt":0.999, "lte":2.0, "lt":2.0}})"_json);
    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == 0);
    REQUIRE(result[1] == 2);
    REQUIRE(result[2] == 3);
  }
  {
    auto result = worker.query_execute(
        R"({"nvalue":{"gt": 1.0, "lt": 2.0}})"_json); // sv_to_v(field.get_range_gt_lt(1.0,
                                                      // 2.0));
    REQUIRE(result.size() == 2);
    REQUIRE(result[0] == 2);
    REQUIRE(result[1] == 3);
  }
  // somewhat smaller upper bounds
  {
    auto result = worker.query_execute(
        R"({"nvalue": {"gt": -1.00, "lte": 0.99}})"_json); // sv_to_v(field.get_range_gt_le(-1.00,
                                                           // 0.99));
    REQUIRE(result.size() == 0);
  }
  {
    auto result =
        worker.query_execute(R"({"nvalue": {"gt":-1.00, "lte": 1.00}})"_json);
    REQUIRE(result.size() == 1);
    auto result2 = worker.query_execute(R"({"nvalue": {"lte": 1.00}})"_json);
    REQUIRE(result2.size() == 1);
  }
  {
    auto result =
        worker.query_execute(R"({"nvalue": {"gt": -1.00, "lt":1.00}})"_json);
    REQUIRE(result.size() == 0);
    auto result2 = worker.query_execute(R"({"nvalue": {"lt":1.00}})"_json);
    REQUIRE(result2.size() == 0);
  }
  // somewhat larger lower bounds
  {
    auto result = worker.query_execute(
        R"({"nvalue": {"gt" : 2.00, "lte" : 10000}})"_json);
    REQUIRE(result.size() == 0);
    auto result_2 = worker.query_execute(R"({"nvalue": {"gt" : 2.00}})"_json);
    REQUIRE(result_2.size() == 0);
    auto result_3 = worker.query_execute(
        R"({"nvalue": {"gte" : 2.00, "lte": 10000}})"_json);
    REQUIRE(result_3.size() == 1);
    auto result_4 = worker.query_execute(R"({"nvalue": {"gte" : 2.00}})"_json);
    REQUIRE(result_4.size() == 1);
  }
  {
    auto result_ = worker.query_execute(R"({"nvalue": {"lte" : 10000}})"_json);
    REQUIRE(result_.size() == 4);
  }
}