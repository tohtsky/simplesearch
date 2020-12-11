cmake_minimum_required(VERSION 2.8.12)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#set(GLIBCXX_DEBUG ON)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -coverage -O0 -std=c++11") # 追加
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -coverage") # 追加
include_directories(include)
add_executable(unit_tests tests/test_numerical.cpp  tests/test_indexer.cpp tests/test_sorted_vector.cpp src/field.cpp src/indexer.cpp src/query.cpp)
target_link_libraries(unit_tests
    gcov
)
include(CTest)
include(CMakeDependentOption)           # This is a really useful scripts that creates options that depends on other options. It can even be used with generator expressions !
include(GNUInstallDirs)                 # This will define the default values for installation directories (all platforms even if named GNU)
include(InstallRequiredSystemLibraries) # Tell CMake that the `install` target needs to install required system libraries (eg: Windows SDK)
include(CMakePackageConfigHelpers)      # Helper to create relocatable packages

add_test(NAME "all" COMMAND ./unit_tests)