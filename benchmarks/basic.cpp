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

   const size_t NUM_ROWS = state.range(0);
   const size_t NUM_COLS = state.range(0) * 2;
   const size_t VEC_LENGTH = NUM_COLS;
   std::vector<float> matrixA(NUM_ROWS * NUM_COLS);
   std::vector<float> vecX(VEC_LENGTH);
   std::vector<float> vecY(VEC_LENGTH);
   std::vector<float> vecY_test(VEC_LENGTH);

   std::iota(std::begin(matrixA), std::end(matrixA), 0);
   std::fill(std::begin(vecX), std::end(vecX), 1);

   for (int r = 0; r < NUM_ROWS; r++)
   {
      tp.submit(
          [&, r]()
          {
             vecY[r] = std::inner_product(matrixA.data() + r * NUM_COLS,
                                          matrixA.data() + r * NUM_COLS + NUM_COLS,
                                          std::begin(vecX), 0.0);
          });
   }

   for (auto _ : state)
   {
      tp.start();
      tp.stop();
   }
   state.SetBytesProcessed(2 * NUM_ROWS * NUM_COLS + NUM_COLS + std::min(NUM_COLS, NUM_ROWS));
}

BENCHMARK(BM_MatrixVector<lock_style::standard_mutex>)->RangeMultiplier(2)->Range(8, 8<<10);

BENCHMARK_MAIN();