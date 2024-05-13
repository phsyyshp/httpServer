
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
class HttpServer {

public:
  HttpServer() {}
  void init() const {

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
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

    send(socket_fd, responseBuffer.data(), responseBuffer.size(), 0);
  }
  void handleConnections(std::string dir) {
    struct sockaddr_in client_addr;

    std::cout << "Waiting for a client to connect...\n";
    int client_addr_len = sizeof(client_addr);
    int socket_fd;
    // //tp
    // ThreadPool pool;
    // for (int i = 0; i < thread::hardware_concurrency() + 1; i++) {
    //   pool.enqueue([socket_fd, dir] { return handleConnection(socket_fd,
    //   dir);
    //   });
    // }

    // std::vector<std::thread> threads;
    while (true) {
      socket_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                         (socklen_t *)&client_addr_len);
      if (socket_fd < 0) {
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
  }
  void closeServer() const { close(server_fd); }

private:
  int server_fd;
  struct sockaddr_in server_addr;
};