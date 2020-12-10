#include "nlohmann/json.hpp"
#include "pybind11_json/pybind11_json.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "invind/indexer.hpp"


namespace py = pybind11;
namespace nl = nlohmann;
using namespace pybind11::literals;
using namespace invind;
PYBIND11_MODULE(invind, m) {
  m.doc() = "My awesome module";
  py::class_<Indexer>(m, "Indexer")
      .def(py::init<>())
      .def("add_categorical_field", &Indexer::add_categorical)
      .def("add_many_to_many_field", &Indexer::add_many_to_many)
      .def("add_index", &Indexer::add_index)
      .def("query_execute", &Indexer::query_execute);
}