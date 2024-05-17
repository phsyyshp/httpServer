#include "httpServer.hpp"

// Class that represents a simple thread pool

std::unordered_map<std::string, std::string> FILE_TYPE_MAP = {
    {".txt", "text/plain"},  {".css", "text/css"},
    {".html", "text/html"},  {".js", "text/javascript"},
    {".apng", "image/apng"}, {".avif", "image/avif"},
    {".gif", "image/gif"},   {".jpg", "image/jpeg"},
    {".png", "image/png"},   {".svg", "image/svg+xml"},
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
