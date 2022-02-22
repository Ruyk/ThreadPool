#include "ThreadPool.h"
#include <thread>
#include <algorithm>
#include <numeric>

#include "benchmark/benchmark.h"


template <lock_style lockStyle>
static void BM_TPCreation(benchmark::State& state) {
    const size_t NUM_THREADS = std::thread::hardware_concurrency();
    ThreadPool<lockStyle> tp{NUM_THREADS*4};
    for (auto _ : state) {
        tp.start();
        tp.stop();
    }
}
// Register the function as a benchmark
BENCHMARK(BM_TPCreation<lock_style::standard_mutex>);
BENCHMARK(BM_TPCreation<lock_style::spin_atomic>);

template <lock_style lockStyle>
static void BM_MatrixVector(benchmark::State& state) {
   const size_t NUM_THREADS = std::thread::hardware_concurrency();
   ThreadPool<lockStyle> tp{NUM_THREADS * 4};

   const size_t NUM_ROWS = 64;
   const size_t NUM_COLS = 128;
   const size_t VEC_LENGTH = NUM_COLS;
   float matrixA[NUM_ROWS * NUM_COLS];
   float vecX[VEC_LENGTH] = {0};
   float vecY[VEC_LENGTH] = {0};
   float vecY_test[VEC_LENGTH] = {0};

   std::iota(std::begin(matrixA), std::end(matrixA), 0);
   std::fill(std::begin(vecX), std::end(vecX), 1);

   for (int r = 0; r < NUM_ROWS; r++)
   {
      tp.submit(
          [&, r]()
          {
             vecY[r] = std::inner_product(matrixA + r * NUM_COLS,
                                          matrixA + r * NUM_COLS + NUM_COLS,
                                          vecX, 0.0);
          });
   }

   for (auto _ : state)
   {
      tp.start();
      tp.stop();
   }
}
BENCHMARK(BM_MatrixVector<lock_style::standard_mutex>);
BENCHMARK(BM_MatrixVector<lock_style::spin_atomic>);

BENCHMARK_MAIN();