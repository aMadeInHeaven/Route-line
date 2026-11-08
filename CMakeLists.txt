cmake_minimum_required(VERSION 3.12)

project(
    labwork7
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(FetchContent)

FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
)

FetchContent_MakeAvailable(cpr)

FetchContent_Declare(json 
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)

FetchContent_MakeAvailable(json)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v6.1.9
)
FetchContent_MakeAvailable(ftxui)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/releases/download/v1.16.0/googletest-1.16.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

set(CTEST_PARALLEL_LEVEL 1)

set(CPR_BUILD_TESTS OFF CACHE BOOL "Disable CPR tests" FORCE)

add_subdirectory(lib)
add_subdirectory(bin)
add_subdirectory(tests)

file(COPY ${CMAKE_SOURCE_DIR}/DataFiles
     DESTINATION ${CMAKE_BINARY_DIR})
# TODO: add your implementation here...

include(tests/coverage.cmake)
