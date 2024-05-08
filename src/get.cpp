#include "get.hpp"

void Getter::get(const RequestLine &requestLine, const std::string &dir) const {
  auto requestTarget = requestLine.requestTarget;
  auto idx = requestLine.requestTarget.find("/files/", 0);
  std::string fileName;
  if (idx != std::string::npos) {
    requestTarget.erase(requestTarget.begin() + idx, requestTarget.begin() + 7);
    fileName = requestTarget;
  }
  std::ifstream file;
  file.open(dir + "/" + fileName);
  std::string startLine;
  std::string body;
  std::string contentLength;
  std::string contentType;
  std::string space = "\r\n";

  if (file) {

    startLine = "HTTP/1.1 200 OK\r\n";
    contentType = "Content-Type: application/octet-stream\r\n";
    while (getline(file, body)) {
      ;
    }
    contentLength = "Content-Length:" + std::to_string(body.length()) + space;
  } else {
    startLine = "HTTP/1.1 404 NOT FOUND\r\n\r\n";
  }
}