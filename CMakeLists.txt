cmake_minimum_required(VERSION 3.2)
set(PROJECT_NAME_STR ThreadPool)
project(${PROJECT_NAME_STR} C CXX)

find_package(Threads REQUIRED)

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


#-------------------
# Tests
#-------------------
add_subdirectory(${EXTERNAL_PROJECTS_DIR}/gtest)
enable_testing()

add_subdirectory(test)
