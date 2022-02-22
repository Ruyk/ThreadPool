set(PROJECT_TEST_NAME ${PROJECT_NAME_STR}_bench)
include_directories(${BENCHMARK_INCLUDE_DIRS} ${COMMON_INCLUDES})

add_executable(bench basic.cpp)
set_property(TARGET bench PROPERTY CXX_STANDARD 17)
target_link_libraries(bench gtest_main)

target_link_libraries(bench ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(bench benchmark::benchmark)

if (CLANG_TIDY_EXE)
  set_property(TARGET ${PROJECT_TEST_NAME} PROPERTY CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
endif()

