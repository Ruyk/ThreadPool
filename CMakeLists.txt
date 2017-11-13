cmake_minimum_required(VERSION 3.2)
set(PROJECT_NAME_STR ThreadPool)
project(${PROJECT_NAME_STR} C CXX)

find_package(Threads REQUIRED)

set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)
set(EXTERNAL_PROJECTS_DIR ${PROJECT_SOURCE_DIR}/external)


#-------------------
# Tests
#-------------------
add_subdirectory(${EXTERNAL_PROJECTS_DIR}/gtest)
enable_testing()

add_subdirectory(test)
