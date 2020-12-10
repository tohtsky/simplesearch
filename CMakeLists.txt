cmake_minimum_required(VERSION 3.0.0)
project(pyinvind VERSION 0.1.0)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(GLIBCXX_DEBUG ON)
include_directories(include)
add_executable(unit_tests tests/test_indexer.cpp tests/test_sorted_vector.cpp src/field.cpp src/indexer.cpp src/query.cpp)

add_subdirectory(pybind11)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

pybind11_add_module(invind src/wrapper.cpp src/field.cpp src/indexer.cpp src/query.cpp)
