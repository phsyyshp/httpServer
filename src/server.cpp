#include "httpServer.hpp"

// Class that represents a simple thread pool
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

std::unordered_map<std::string, std::string> FILE_TYPE_MAP = {
    {

        ".txt", "text/plain"},
    {".css", "text/css"},
    {".html", "text/html"},
    {".js", "text/javascript"},
    {".apng", "image/apng"},
    {".avif", "image/avif"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".png", "image/png"},
    {".svg", "image/svg+xml"},
    {".webp", "image/webp"}

};
std::mutex cout_mutex;
int main(int argc, char **argv) {
  std::string dir = "";
  if (argc > 2) {
    dir = argv[2];
    // std::cout << dir;
  }
  std::cout << "Logs from your program will appear here!\n";

  HttpServer server;
  server.init();
  server.bindPort();
  server.listenConnection();
  server.handleConnections(dir);
  server.closeServer();
  return 0;
}
