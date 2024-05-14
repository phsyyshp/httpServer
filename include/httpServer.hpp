
#include "request.hpp"
#include "response.hpp"
#include <arpa/inet.h>
#include <array>
#include <cerrno> // For errno
#include <cstdlib>
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
class HttpServer {

public:
  HttpServer() {}
  void init() {

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
      std::cerr << "Failed to create server socket\n";
    }
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) <
        0) {
      std::cerr << "setsockopt failed\n";
    }
  }

  void bindPort(int port = 4221) {

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    // server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
        0) {
      std::cerr << "Failed to bind to port " + std::to_string(port) + "\n";
    }
  }
  void listenConnection() const {

    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0) {
      std::cerr << "listen failed\n";
      // return 1;
    }
  }

  void handleConnection(int socket_fd, std::string dir) {
    if (socket_fd < 0) {
      std::cerr << "Invalid socket file descriptor\n";
      return; // E
    }
    std::cout << "Client connected\n";

    /*
    RFC 9112:
    HTTP/1.1 defaults to the use of "persistent connections", allowing
    multiple requests and responses to be carried over a single connection.
    HTTP implementations SHOULD support persistent connections.
    A recipient determines whether a connection is persistent or not based on
    the protocol version and Connection header field (Section 7.6.1 of [HTTP])
    in the most recently received message, if any:
    */
    bool localKeepAlive = true;
    while (localKeepAlive) {

      std::array<char, 1024> buffer;
      int valread = read(socket_fd, buffer.data(), sizeof(buffer) - 1);

      if (valread < 0) {
        std::cerr << "Error reading from socket\n";
        close(socket_fd);
        return; // Handle read error
      } else if (valread == 0) {
        close(socket_fd);
        return;
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

      send(socket_fd, responseBuffer.data(), responseBuffer.size(), 0);
      if (request.getHeaderHash()["Connection"] == "close") {
        /*RFC: 9112
          If the "close" connection option is present (Section 9.6), the
          connection will not persist after the current response; else, If the
          received protocol is HTTP/1.1 (or later), the connection will persist
          after the current response;
        */
        localKeepAlive = false;
        close(socket_fd);
      }
    }
  }
  void handleConnections(std::string dir) {
    struct sockaddr_in client_addr;

    std::cout << "Waiting for a client to connect...\n";
    int client_addr_len = sizeof(client_addr);
    int socket_fd;
    // //tp
    while (true) {

      client_addr_len = sizeof(client_addr);
      int socket_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                             (socklen_t *)&client_addr_len);
      if (socket_fd < 0) {
        std::cerr << "Failed to accept connection\n";
        continue;
      }
      pool.enqueue(
          [socket_fd, dir, this] { return handleConnection(socket_fd, dir); });
    }
  }
  void closeServer() const { close(server_fd); }

private:
  int server_fd;
  struct sockaddr_in server_addr;
  ThreadPool pool;
  std::mutex keepAliveMutex_;
  bool keepAlive = true;
};