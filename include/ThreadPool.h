/**
 * C++11 Thread Pool Implementation
 *
 * Author: Ruyman Reyes <ruyman@codeplay.com>
 *
 *
 */

/** ThreadPool.
 *
 */
class ThreadPool {
 public:
  ThreadPool(size_t numThreads) : nT_{numThreads} {};

  ~ThreadPool() = default;

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool(ThreadPool &&) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;
  ThreadPool &operator=(ThreadPool &&) = delete;

  /** get_num_threads.
   *   return Returns the number of threads
   */
  inline size_t get_num_threads() const { return nT_; }

 private:
  size_t nT_;
};
