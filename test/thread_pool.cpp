#include "ThreadPool.h"
#include "gtest/gtest.h"

TEST(ThreadPooltest, Constructor) {
  const size_t NUM_THREADS = 3lu;
  ThreadPool tp{NUM_THREADS};

  ASSERT_EQ(tp.get_num_threads(), NUM_THREADS);
  ASSERT_FALSE(tp.is_running());

  ASSERT_EQ(tp.pending_tasks(), 0);
}

TEST(ThreadPooltest, ConstructorInvalid) {
  const size_t NUM_THREADS = 0lu;
  ASSERT_THROW(
        ThreadPool tp{NUM_THREADS},
        std::logic_error);
}

TEST(ThreadPooltest, StartStop) {
  const size_t NUM_THREADS = std::thread::hardware_concurrency();
  ThreadPool tp{NUM_THREADS};

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

TEST(ThreadPooltest, RunSingleTask) {
  auto testSingleTask = [&](size_t NUM_THREADS) {
    std::string scopedTraceName = "Single Task with " + std::to_string(NUM_THREADS);

    SCOPED_TRACE(scopedTraceName);
    ThreadPool tp{NUM_THREADS};

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

TEST(ThreadPooltest, RunMultipleTasks) {
  const size_t NUM_THREADS = std::thread::hardware_concurrency();
  const size_t NUM_TASKS = 1256;
  ThreadPool tp{NUM_THREADS};
  

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

TEST(ThreadPooltest, WaitForAll) {
  const size_t NUM_THREADS = std::thread::hardware_concurrency();
  const size_t NUM_TASKS = 1256;
  ThreadPool tp{NUM_THREADS};
  std::atomic<int> count{0};
  std::atomic<int> waitingCount{0};
  volatile std::atomic<bool> ready{false};
  std::mutex waitToContinue;

  auto counterTask = [&]() { 
    bool quit = false;
    waitingCount++;
    while (quit) {
      std::this_thread::yield();
      waitToContinue.lock();
      if (ready) {
        quit = true;
      }
      waitToContinue.unlock();
    }
    count++;
  };
  tp.start();

  for (int i = 0; i < NUM_TASKS; i++) {
    tp.submit(counterTask);
  }

  while (waitingCount < static_cast<int>(NUM_TASKS)) { 
    std::this_thread::yield();
  };

  ASSERT_EQ(waitingCount, NUM_TASKS);
  ASSERT_EQ(tp.pending_tasks(), NUM_TASKS);
  waitToContinue.lock();
  ready = true;
  waitToContinue.unlock();

  tp.wait_for_all_pending_tasks();

  ASSERT_EQ(tp.pending_tasks(), 0);

  tp.stop();

  ASSERT_EQ(tp.pending_tasks(), 0);

  ASSERT_FALSE(tp.is_running());

  ASSERT_EQ(count, NUM_TASKS);

}

TEST(ThreadPooltest, RunMultipleRandomDurationTasks) {
  const size_t NUM_THREADS = std::thread::hardware_concurrency();
  const size_t NUM_TASKS = 156;
  ThreadPool tp{NUM_THREADS};
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
