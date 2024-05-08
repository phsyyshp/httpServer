#include "response.hpp"
std::string Response::respond(const Request &request) const {

  std::string out;
  std::string startLine;
  std::unordered_map<std::string, std::string> responseMap;
  std::string path = request.parseRequestLine()["path"];
  std::string textToEcho;

  startLine = "HTTP/1.1 200 OK\r\n";
  auto idx = path.find("/echo/", 0);
  std::string space = "\r\n";
  if (idx != std::string::npos) {
    path.erase(path.begin() + idx, path.begin() + 6);
    std::string contentType = "Content-Type: text/plain\r\n";
    auto body = path;
    std::string contentLength =
        "Content-Length:" + std::to_string(body.length()) + space;

    out = startLine + contentType + contentLength + "\r\n" + body;
  } else if (request.parseRequestLine()["path"] == "/") {
    out = startLine + space;

  } else {
    out = "HTTP/1.1 404 NOT FOUND\r\n\r\n";
  }
  return out;
}
