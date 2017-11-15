/**
  Copyright 2017 Ruyman Reyes
  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at
      http://www.apache.org/licenses/LICENSE-2.0
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

   file: ThreadPool.h
   author: Ruyman Reyes <ruyman@codeplay.com>
*/

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

#include <iostream>

#define VERBOSE 0
#define EXCEPTION_SUPPORT 1

#if VERBOSE
#define VERBOSE_COUT std::cout
#else
#define VERBOSE_COUT                                                           \
   if (1) {                                                                    \
   }                                                                           \
   else                                                                        \
      std::cout
#endif

/** ThreadPool.
 * A static thread pool implementation using C++11 threads.
 *
 */
class ThreadPool
{
   public:
   ThreadPool() noexcept = delete;

   ThreadPool(size_t numThreads)
       : nT_{numThreads}
       , running_{false}
       , pendingTasksMutex_{}
       , pendingTaskCondition_{}
       , workers_{}
       , pendingTasks_{}
   {
      workers_.reserve(std::thread::hardware_concurrency());
      if (numThreads == 0) {
         throw std::logic_error("Cannot create a thread pool with no threads");
      }
   };

   ~ThreadPool() {
     if (is_running()) {
       stop();
     }
   }
   ThreadPool(const ThreadPool&) = delete;
   ThreadPool(ThreadPool&&) = delete;
   ThreadPool& operator=(const ThreadPool&) = delete;
   ThreadPool& operator=(ThreadPool&&) = delete;

   /**
    * Starts the worker threads from the thread pool.
    */
   void start()
   {
      if (running_) {
         return;
      }
      VERBOSE_COUT << " Start workers " << std::endl;
      running_ = true;
      for (size_t i = 0ul; i < nT_; i++) {
         auto runner = [this] {
            VERBOSE_COUT << " Thread " << std::this_thread::get_id()
                         << " starts " << std::endl;
            while (1) {
               VERBOSE_COUT << " Running loop " << std::endl;
               std::function<void()> func;
               {
                  std::unique_lock<std::mutex> pendingTasksLock(
                      pendingTasksMutex_);
                  pendingTaskCondition_.wait(pendingTasksLock, [this] {
                     return (!this->pendingTasks_.empty() ||
                             !this->is_running());
                  });
                  if (!pendingTasks_.empty()) {
                     VERBOSE_COUT << " Obtains a Task "
                                  << std::this_thread::get_id() << std::endl;
                     func = std::move(this->pendingTasks_.front());
                     pendingTasks_.pop();
                  }
                  else if (pendingTasks_.empty() && !this->is_running())
                  {
                     VERBOSE_COUT << " Finish running "
                                  << std::this_thread::get_id() << std::endl;
                     break;
                  }
                  else if (pendingTasks_.empty() && this->is_running())
                  {
                     // Should not happen
                     VERBOSE_COUT << " Cannot happen " << std::endl;
                  }
                  else
                  {
                     VERBOSE_COUT << " Unreachable " << std::endl;
                     VERBOSE_COUT << " Is running " << is_running()
                                  << std::endl;
                     VERBOSE_COUT << " Pending task count "
                                  << pendingTasks_.size() << std::endl;
#if EXCEPTION_SUPPORT
                     throw std::runtime_error("Unreachable");
#endif // EXCEPTION_SUPPORT
                  }
               } // pendingTasksLock
               VERBOSE_COUT << " Start Task " << std::this_thread::get_id()
                            << std::endl;
               if (func) {
                  func();
               }
               waitingCondition_.notify_one();
            } // while running
         };   // runner
         workers_.emplace_back(runner);
      } // for
   }    // start

   /**
    * Submits a functor for execution on the Thread Pool
    */
   template <class Functor>
   bool submit(Functor&& f)
   {
      VERBOSE_COUT << " Submit a task " << std::endl;
      bool retStatus = true;
      if (!is_running()) {
         retStatus = false;
      }
      try
      {
         {
            std::unique_lock<std::mutex> pendingTasksLock(pendingTasksMutex_);
            pendingTasks_.emplace(f);
         }
         pendingTaskCondition_.notify_one();
      }
      catch (...)
      {
         retStatus = false;
#if EXCEPTION_SUPPORT
         throw;
#endif
      }
      return retStatus;
   }

   /** stop.
    * Stops all running worker threads.
    * If there are threads still running, waits until they are completed.
    */
   void stop()
   {
      if (!running_) {
         return;
      }
      VERBOSE_COUT << " Stop workers " << std::endl;

      wait_for_all_pending_tasks();

      {
         std::unique_lock<std::mutex> pendingTasksLock(pendingTasksMutex_);
         running_ = false;
      }
      pendingTaskCondition_.notify_all();
      for (auto& w : workers_) {
         if (w.joinable()) {
            w.join();
         }
      }
   }

   /** get_num_threads.
    *   return Returns the number of threads
    */
   inline size_t get_num_threads() const { return nT_; }

   /**
    * Returns true if the Thread Pool has workers running
    */
   inline bool is_running() const { return running_; }

   /**
    * Returns the number of pending tasks to execute on the
    * thread pool.
    */
   inline size_t pending_tasks() const
   {
      return pendingTasks_.size();
   }

   /**
    * Waits for all pending tasks before returning.
    */
   inline void wait_for_all_pending_tasks()
   {
      if (!is_running()) return;
      {
         VERBOSE_COUT << " Waiting for tasks to finish " << std::endl;
         std::unique_lock<std::mutex> pendingTasksLock(pendingTasksMutex_);
         waitingCondition_.wait(
             pendingTasksLock, [this] { return this->pendingTasks_.empty(); });
         VERBOSE_COUT << " After waiting for tasks to complete " << std::endl;
      }
   }

   private:
   size_t nT_{};
   std::atomic<bool> running_;
   std::mutex pendingTasksMutex_;
   std::condition_variable pendingTaskCondition_;
   std::condition_variable waitingCondition_;
   std::queue<std::function<void()>> pendingTasks_;
   std::vector<std::thread> workers_;
};

#endif // THREAD_POOL_H
