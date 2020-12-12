#include "nlohmann/json.hpp"
#include "pybind11_json/pybind11_json.hpp"
#include "simplesearch/indexer.hpp"
#include "simplesearch/sorted_vector.hpp"
#include <atomic>
#include <future>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
namespace nl = nlohmann;
using namespace pybind11::literals;
using namespace simplesearch;

template <typename T> py::array_t<T> vector_to_np(const std::vector<T> &v) {
  py::array_t<T> np_result(v.size());
  if (v.empty()) {
    return np_result;
  }
  auto buf = np_result.request();
  std::memcpy(buf.ptr, static_cast<const void *>(v.data()),
              v.size() * sizeof(T));
  return np_result;
}

PYBIND11_MODULE(simplesearch, m) {
  m.doc() = "My awesome module";
  py::class_<Indexer>(m, "Indexer")
      .def(py::init<>())
      .def("add_categorical_field", &Indexer::add_categorical)
      .def("add_many_to_many_field", &Indexer::add_many_to_many)
      .def("add_numerical_field", &Indexer::add_numerical)
      .def("add_index", &Indexer::add_index)
      .def("query_execute",
           [](const Indexer &indexer, const nl::json &query) {
             auto result_vector = indexer.query_execute(query);
             return vector_to_np(result_vector);
           })
      .def("query_execute_batch",
           [](const Indexer &indexer, const std::vector<nl::json> &queries,
              size_t n_thread) {
             using nptype = py::array_t<uint64_t>;
             std::atomic<uint64_t> current{static_cast<uint64_t>(0)};
             auto working_function = [&indexer, &queries, &current]() {
               std::vector<std::pair<uint64_t, std::vector<uint64_t>>> results;
               while (true) {
                 uint64_t index = current++;
                 if (index >= queries.size())
                   break;
                 results.emplace_back(index,
                                      indexer.query_execute(queries[index]));
               }
               return results;
             };
             std::vector<std::future<
                 std::vector<std::pair<uint64_t, std::vector<uint64_t>>>>>
                 worker_results;
             for (uint64_t th = 0; th < n_thread; th++) {
               worker_results.emplace_back(
                   std::async(std::launch::async, working_function));
             }
             std::vector<nptype> results(queries.size());
             for (auto &th_res : worker_results) {
               auto th_result_got = th_res.get();
               for (auto &ind_np : th_result_got) {
                 results[ind_np.first] = vector_to_np(ind_np.second);
               }
             }
             return results;
           })
      .def("__repr__", &Indexer::repr);
}