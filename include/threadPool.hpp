#pragma once
#include "request.hpp"
#include "response.hpp"
#include <arpa/inet.h>
#include <array>
#include <cerrno> // For errno
#include <condition_variable>
#include <cstdlib>
#include <cstring> // For strerror
#include <fcntl.h> // For F_GETFL, F_SETFL
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <queue>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
// This class is taken from https://www.geeksforgeeks.org/thread-pool-in-cpp/
//  C++ Program to demonstrate thread pooling

class ThreadPool {
public:
  // // Constructor to creates a thread pool with given
  // number of threads
  ThreadPool(size_t num_threads = std::thread::hardware_concurrency()) {

    // Creating worker threads
    for (size_t i = 0; i < num_threads; ++i) {
      threads_.emplace_back([this] {
        while (true) {
          std::function<void()> task;
          // The reason for putting the below code
          // here is to unlock the queue before
          // executing the task so that other
          // threads can perform enqueue tasks
          {
            // Locking the queue so that data
            // can be shared safely
            std::unique_lock<std::mutex> lock(queue_mutex_);

            // Waiting until there is a task to
            // execute or the pool is stopped
            cv_.wait(lock, [this] { return !tasks_.empty() || stop_; });

            // exit the thread in case the pool
            // is stopped and there are no tasks
            if (stop_ && tasks_.empty()) {
              return;
            }

            // Get the next task from the queue
            task = move(tasks_.front());
            tasks_.pop();
          }

          task();
        }
      });
    }
  }

  // Destructor to stop the thread pool
  ~ThreadPool() {
    {
      // Lock the queue to update the stop flag safely
      std::unique_lock<std::mutex> lock(queue_mutex_);
      stop_ = true;
    }

    // Notify all threads
    cv_.notify_all();

    // Joining all worker threads to ensure they have
    // completed their tasks
    for (auto &thread : threads_) {
      thread.join();
    }
  }

  // Enqueue task for execution by the thread pool
  void enqueue(std::function<void()> task) {
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      tasks_.emplace(move(task));
    }
    cv_.notify_one();
  }

private:
  // Vector to store worker threads
  std::vector<std::thread> threads_;

  // Queue of tasks
  std::queue<std::function<void()>> tasks_;

  // Mutex to synchronize access to shared data
  std::mutex queue_mutex_;

  // Condition variable to signal changes in the state of
  // the tasks queue
  std::condition_variable cv_;

  // Flag to indicate whether the thread pool should stop
  // or not
  bool stop_ = false;
};