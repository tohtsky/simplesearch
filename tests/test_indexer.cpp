#include "catch.hpp"
#include "simplesearch/indexer.hpp"
#include <stdexcept>

#include <iostream>

using namespace simplesearch;
TEST_CASE("basic", "[basic]") {
  Indexer worker;

  worker.add_categorical("gender");
  worker.add_categorical("location");
  worker.add_many_to_many("job_categories");

  json data_1 =
      R"({"gender": "M", "location": "K01", "job_categories": ["1", "2", "3"]})"_json;
  json data_2 =
      R"({"gender": "F", "location": "K01", "job_categories": ["1"]})"_json;
  json data_3 =
      R"({"gender": "M", "location": "J02", "job_categories": null})"_json;
  json data_4 =
      R"({"gender": null, "location": "J02", "job_categories":["2", "3"]})"_json; // bad
                                                                                  // type
                                                                                  // for gender

  worker.add_index(data_1);
  worker.add_index(data_2);
  worker.add_index(data_3);
  REQUIRE(worker.size() == 3);
  worker.add_index(data_4);

  {

    auto result_1 = worker.query_execute(R"({"gender": "M"})"_json);
    REQUIRE(result_1.size() == 2);

    REQUIRE(result_1.at(0) == 0);
    REQUIRE(result_1.at(1) == 2);
  }
  {
    auto result_2 = worker.query_execute(R"({})"_json);
    REQUIRE(result_2.size() == 4);
  }
  {
    auto result_3 =
        worker.query_execute(R"({"gender": "M", "location":"J02"})"_json);
    REQUIRE(result_3.size() == 1);
    REQUIRE(result_3.at(0) == 2);
  }
  {
    auto result = worker.query_execute(R"({"job_categories": ["1"]})"_json);
    REQUIRE(result.size() == 2);
    REQUIRE(result.at(0) == 0);
    REQUIRE(result.at(1) == 1);
  }
  {
    auto result = worker.query_execute(
        R"({"job_categories": {"contains_all": ["2", "3"]}})"_json);
    REQUIRE(result.size() == 2);
    REQUIRE(result.at(0) == 0);
    REQUIRE(result.at(1) == 3);
  }
  {
    auto result = worker.query_execute(
        R"({"gender": ["M"], "location": ["K01", "J02"], "job_categories": {"contains_one": ["2", "1"]}})"_json);
    REQUIRE(result.size() == 1);
    REQUIRE(result.at(0) == 0);
  }
  {
    auto result = worker.query_execute(R"({"gender": "L"})"_json);
    REQUIRE(result.size() == 0);
  }
  {
    auto result = worker.query_execute(R"({"gender": null})"_json);
    REQUIRE(result.size() == 1);
  }
  {
    auto result = worker.query_execute(
        R"({"or_scope": {"gender":"F", "job_categories": {"contains_all": ["1", "2", "3"]}}})"_json);
    REQUIRE(result.size() == 2);
    REQUIRE(result[0] == 0);
    REQUIRE(result[1] == 1);
  }
  {
    auto result = worker.query_execute(
        R"({"and_scope": {"job_categories": ["3"]}, "or_scope": {"gender":"F", "job_categories": {"contains_all": ["1", "2", "3"]}}})"_json);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 0);
  }
};