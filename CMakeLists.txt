cmake_minimum_required(VERSION 3.6)
set(PROJECT_NAME_STR ThreadPool)
project(${PROJECT_NAME_STR} C CXX)

find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Fetch Google Benchmark library
FetchContent_Declare(
  gbench
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/benchmark/archive/refs/tags/v1.6.1.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(gbench)

set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)
set(EXTERNAL_PROJECTS_DIR ${PROJECT_SOURCE_DIR}/external)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(
  CLANG_TIDY_EXE
  NAMES "clang-tidy"
  DOC "Path to the clang-tidy executable"
  )

if(NOT CLANG_TIDY_EXE)
  message(STATUS "clang-tidy not found")
else()
  message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
  set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=*,-clang-analyzer-alpha.*,-google-runtime-int -header-filter=ThreadPool.h")
endif()

add_subdirectory(benchmarks)

enable_testing()
add_subdirectory(test)
