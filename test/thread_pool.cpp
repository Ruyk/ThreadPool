#include <algorithm>

#include "ThreadPool.h"
#include "gtest/gtest.h"

TEST(ThreadPooltest, Constructor) {
  const size_t NUM_THREADS = 3lu;
  ThreadPool tp{NUM_THREADS};

  ASSERT_EQ(tp.get_num_threads(), NUM_THREADS);
}
