#include "request.hpp"
#include "response.hpp"
#include <arpa/inet.h>
#include <array>
#include <cerrno> // For errno
#include <cstdlib>
#include <cstring>
#include <cstring> // For strerror
#include <fcntl.h> // For F_GETFL, F_SETFL
#include <future>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <unistd.h> // For fcntl

// C++ Program to demonstrate thread pooling

#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
using namespace std;

// Class that represents a simple thread pool
class ThreadPool {
public:
  // // Constructor to creates a thread pool with given
  // number of threads
  ThreadPool(size_t num_threads = thread::hardware_concurrency()) {

    // Creating worker threads
    for (size_t i = 0; i < num_threads; ++i) {
      threads_.emplace_back([this] {
        while (true) {
          function<void()> task;
          // The reason for putting the below code
          // here is to unlock the queue before
          // executing the task so that other
          // threads can perform enqueue tasks
          {
            // Locking the queue so that data
            // can be shared safely
            unique_lock<mutex> lock(queue_mutex_);

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
      unique_lock<mutex> lock(queue_mutex_);
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
  void enqueue(function<void()> task) {
    {
      unique_lock<std::mutex> lock(queue_mutex_);
      tasks_.emplace(move(task));
    }
    cv_.notify_one();
  }

private:
  // Vector to store worker threads
  vector<thread> threads_;

  // Queue of tasks
  queue<function<void()>> tasks_;

  // Mutex to synchronize access to shared data
  mutex queue_mutex_;

  // Condition variable to signal changes in the state of
  // the tasks queue
  condition_variable cv_;

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
void handleConnection(int socket_fd, std::string dir) {

  if (socket_fd < 0) {
    std::cerr << "Invalid socket file descriptor\n";
    return; // E
  }
  // std::cout << "Client connected\n";
  std::array<char, 1024> buffer;
  int valread = read(socket_fd, buffer.data(), sizeof(buffer) - 1);
  if (valread < 0) {
    std::cerr << "Error reading from socket\n";
    return; // Handle read error
  }
  buffer[valread] = '\0'; // Ensure null-termination
  Request request;
  std::string responseBuffer;
  Response response;

  if (request.parse(buffer)) {

    responseBuffer = response.respond(request, dir);
  } else {
    responseBuffer = response.badRequest(request);
  }

  // std::cout << responseBuffer;

  send(socket_fd, responseBuffer.data(), responseBuffer.size(), 0);
}
int main(int argc, char **argv) {
  std::string dir = "";
  if (argc > 2) {
    dir = argv[2];
    // std::cout << dir;
  }
  // You can use print statements as follows for debugging, they'll be visible
  // when running tests.
  std::cout << "Logs from your program will appear here!\n";

  // Uncomment this block to pass the first stage
  //
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Failed to create server socket\n";
    return 1;
  }
  // non block
  // int flags = fcntl(server_fd, F_GETFL, 0);
  // if (flags < 0) {
  //   std::cerr << "Failed to get flags: " << strerror(errno) << std::endl;
  //   return -1;
  // }
  // if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
  //   std::cerr << "Failed to set non-blocking: " << strerror(errno) <<
  //   std::endl; return -1;
  // }

  // Since the tester restarts your program quite often, setting REUSE_PORT
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) <
      0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  // server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
      0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    // return 1;
    return 1;
  }

  struct sockaddr_in client_addr;

  std::cout << "Waiting for a client to connect...\n";
  int client_addr_len = sizeof(client_addr);
  int socket_fd;
  // //tp
  // ThreadPool pool;
  // for (int i = 0; i < thread::hardware_concurrency() + 1; i++) {
  //   pool.enqueue([socket_fd, dir] { return handleConnection(socket_fd, dir);
  //   });
  // }

  std::vector<std::thread> threads;
  while (true) {
    socket_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                       (socklen_t *)&client_addr_len);
    if (socket_fd < 0) {
      // if (errno == EAGAIN || errno == EWOULDBLOCK) {
      //   // No pending connections; you could add a sleep here to prevent busy
      //   // waiting
      //   continue;
      // } else {
      //   std::cerr << "Failed to accept: " << strerror(errno) << std::endl;
      //   break;
      // }
      std::cerr << "Failed to accept connection\n";
      continue;
    }
    handleConnection(socket_fd, dir);
    // threads.emplace_back(handleConnection, socket_fd, dir);
  }

  // for (auto &t : threads) {
  //   if (t.joinable()) {
  //     t.join();
  //   }
  // }
  close(server_fd);
  // std::cout << "lalal";
  return 0;
}
