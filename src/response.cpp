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
  std::string space = "\r\n";
  startLine = "HTTP/1.1 200 OK\r\n";
  std::string contentType = "Content-Type: text/plain\r\n";
  auto body = path;
  std::string contentLength =
      "Content-Length:" + std::to_string(body.length()) + space;

  out = startLine + contentType + contentLength + "\r\n" + body;
  //   if (request.parseRequestLine()["path"] == "/") {
  //   } else {
  //     startLine = "HTTP/1.1 404 NOT FOUND\r\n\r\n";
  //   }
  //   retrun
  return out;
}
