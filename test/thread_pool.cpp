#include "ThreadPool.h"
#include "gtest/gtest.h"

template <typename T>
class ThreadPooltest : public ::testing::Test {
};

typedef ::testing::Types</* wait_style<lock_style::standard_mutex>,*/
                         wait_style<lock_style::spin_atomic>> MyTypes;
TYPED_TEST_CASE(ThreadPooltest, MyTypes);

TYPED_TEST(ThreadPooltest, Constructor) {
  const size_t NUM_THREADS = 3lu;
  ThreadPool<TypeParam::style> tp{NUM_THREADS};

  ASSERT_EQ(tp.get_num_threads(), NUM_THREADS);
  ASSERT_FALSE(tp.is_running());

  ASSERT_EQ(tp.pending_tasks(), 0); 
}

TYPED_TEST(ThreadPooltest, ConstructorInvalid) {
  const size_t NUM_THREADS = 0lu;
  ASSERT_THROW(
        ThreadPool<TypeParam::style> tp{NUM_THREADS},
        std::logic_error);
}

TYPED_TEST(ThreadPooltest, StartStop) {
  const size_t NUM_THREADS = std::thread::hardware_concurrency();
  ThreadPool<TypeParam::style> tp{NUM_THREADS};

  ASSERT_EQ(tp.get_num_threads(), NUM_THREADS);

  ASSERT_FALSE(tp.is_running());

  ASSERT_EQ(tp.pending_tasks(), 0);

  tp.start();

  ASSERT_TRUE(tp.is_running());

  tp.stop();

  ASSERT_FALSE(tp.is_running());

  // A second stop should not change anything
  tp.stop();

  ASSERT_FALSE(tp.is_running());

}

TYPED_TEST(ThreadPooltest, RunSingleTask) {
  auto testSingleTask = [&](size_t NUM_THREADS) {
    std::string scopedTraceName = "Single Task with " + std::to_string(NUM_THREADS);

    SCOPED_TRACE(scopedTraceName);
    ThreadPool<TypeParam::style> tp{NUM_THREADS};

    ASSERT_EQ(tp.get_num_threads(), NUM_THREADS);
    ASSERT_FALSE(tp.is_running());

    int count = 0;

    auto singleTask = [&]() { 
      count++;
    };

    tp.start();

    ASSERT_TRUE(tp.is_running());

    ASSERT_EQ(tp.pending_tasks(), 0);
    tp.submit(singleTask);

    tp.stop();

    ASSERT_EQ(tp.pending_tasks(), 0);

    ASSERT_FALSE(tp.is_running());

    ASSERT_EQ(count, 1);
  };

  for (size_t nT = 1; nT < 2*std::thread::hardware_concurrency() ; nT++) {
    VERBOSE_COUT << "---------------------" << std::endl;
    testSingleTask(nT);
    VERBOSE_COUT << "---------------------" << std::endl;
  }
}

TYPED_TEST(ThreadPooltest, RunMultipleTasks) {
  const size_t NUM_THREADS = std::thread::hardware_concurrency();
  const size_t NUM_TASKS = 1256;
  ThreadPool<TypeParam::style> tp{NUM_THREADS};
  

  ASSERT_FALSE(tp.is_running());

  std::atomic<int> count{0};

  auto counterTask = [&]() { 
    count++;
  };

  tp.start();

  ASSERT_TRUE(tp.is_running());

  ASSERT_EQ(tp.pending_tasks(), 0);

  for (int i = 0; i < NUM_TASKS; i++) {
    tp.submit(counterTask);
  }

  tp.stop();

  ASSERT_EQ(tp.pending_tasks(), 0);

  ASSERT_FALSE(tp.is_running());

  ASSERT_EQ(count, NUM_TASKS);

}

TYPED_TEST(ThreadPooltest, WaitForAll) {
  const size_t NUM_THREADS = std::thread::hardware_concurrency();
  const size_t NUM_TASKS = NUM_THREADS + 1;
  ThreadPool<TypeParam::style> tp{NUM_THREADS};

  std::atomic<int> finishedExecutionCount{0};
  std::atomic<int> waitingCount{0};
  volatile std::atomic<bool> ready{false};

  // This counter task will spin loop waiting for 
  // ready to be set to true. 
  // All worker threads will simply get stuck there.
  // Note that no new threads will be picked so
  // until ready is true, only NUM_THREADS tasks will be
  // actively waiting
  auto counterTask = [&]() { 
    waitingCount++;
    while (!ready) { }
    finishedExecutionCount++;
  };
  tp.start();

  for (int i = 0; i < NUM_TASKS; i++) {
    tp.submit(counterTask);
  }

  // Wait until all threads have picked up a task
  // Note that there is an extra task that will not be
  // picked by any thread, so it will not be stuck in the waiting loop.
  while (waitingCount < NUM_THREADS) { 
    std::this_thread::yield();
  };

  ASSERT_GE(tp.pending_tasks(), NUM_TASKS - 1);

  ready = true;

  tp.wait_for_all_pending_tasks();

  ASSERT_EQ(tp.pending_tasks(), 0);

  tp.stop();

  ASSERT_EQ(tp.pending_tasks(), 0);

  ASSERT_FALSE(tp.is_running());

  ASSERT_EQ(finishedExecutionCount, NUM_TASKS);
}

TYPED_TEST(ThreadPooltest, RunMultipleRandomDurationTasks) {
  const size_t NUM_THREADS = std::thread::hardware_concurrency();
  const size_t NUM_TASKS = 156;
  ThreadPool<TypeParam::style> tp{NUM_THREADS};
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_real_distribution<double> dist(1.0, 80.0);

  ASSERT_FALSE(tp.is_running());

  std::atomic<int> count{0};

  auto counterTask = [&]() { 
    std::this_thread::sleep_for(
          std::chrono::duration<double, std::milli>(dist(mt)));
    count++;
  };

  tp.start();

  ASSERT_TRUE(tp.is_running());

  ASSERT_EQ(tp.pending_tasks(), 0);

  for (int i = 0; i < NUM_TASKS; i++) {
    tp.submit(counterTask);
  }

  tp.stop();

  ASSERT_EQ(tp.pending_tasks(), 0);

  ASSERT_FALSE(tp.is_running());

  ASSERT_EQ(count, NUM_TASKS);

}
