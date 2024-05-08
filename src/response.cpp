#include "response.hpp"
std::string Response::respond(const Request &request) const {

  std::string out;
  std::string startLine;
  std::unordered_map<std::string, std::string> responseMap;
  std::string path = request.parseRequestLine()["path"];
  std::string textToEcho;
  auto idx = path.find("/echo/", 0);
  if (idx != std::string::npos) {
    path.erase(path.begin() + idx, path.begin() + 6);
  }
  std::string space = "\r\n\r\n";
  startLine = "HTTP/1.1 200 OK\r\n\r\n";
  std::string contentType = "Content-Type: text/plain\r\n\r\n";
  std::string contentLength =
      "Content-Length:" + std::to_string(path.length()) + space;
  auto body = path + " \r\n";

  out = startLine + contentType + contentLength + "\r\n\r\n" + path;
  //   if (request.parseRequestLine()["path"] == "/") {
  //   } else {
  //     startLine = "HTTP/1.1 404 NOT FOUND\r\n\r\n";
  //   }
  //   retrun
  return out;
}
