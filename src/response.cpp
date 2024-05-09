#include "response.hpp"
std::string Response::respond(const Request &request,
                              const std::string &dir) const {

  std::string out;
  std::string body;
  std::string startLine;
  std::string contentLength;
  std::string contentType;
  std::string line;
  RequestLine requestLine = request.getRequestLine();
  std::string requestTarget = requestLine.requestTarget;
  std::string textToEcho;
  std::string space = "\r\n";
  startLine = "HTTP/1.1 200 OK\r\n";
  auto idx = requestTarget.find("/echo/", 0);
  auto idx2 = requestLine.requestTarget.find("/files/", 0);
  std::string fileName;

  // std::cout << "halala";
  if (requestLine.requestTarget == "/") {
    contentType = "Content-Type: text/plain\r\n";
    contentLength = "Content-Length:" + std::to_string(4) + space;
    return startLine + contentType + contentLength + "Connection: close\r\n" +
           "\r\n" + "pass";
  }

  if (idx != std::string::npos) {
    requestTarget.erase(requestTarget.begin() + idx, requestTarget.begin() + 6);
    body = requestTarget;
  } else if (requestLine.requestTarget == "/user-agent") {
    body = request.getHeaderHash()["User-Agent"];
  } else if (idx2 != std::string::npos) {
    requestTarget.erase(requestTarget.begin() + idx2,
                        requestTarget.begin() + 7);
    fileName = requestTarget;
    std::ifstream file;
    file.open(dir + "/" + fileName);
    std::cout << fileName;
    if (!file) {
      startLine = "HTTP/1.1 404 NOT FOUND\r\n";
      contentType = "Content-Type: text/plain\r\n";
      contentLength = "Content-Length:" + std::to_string(3) + space;
      return startLine + contentType + contentLength + "Connection: close\r\n" +
             "\r\n" + "err";
    }

    startLine = "HTTP/1.1 200 OK\r\n";
    contentType = "Content-Type: application/octet-stream\r\n";
    while (getline(file, line)) {
      body += line;
    }
    contentLength = "Content-Length:" + std::to_string(body.length()) + space;
    return startLine + contentType + contentLength + "\r\n" + body;

  } else {
    startLine = "HTTP/1.1 404 NOT FOUND\r\n";
    contentType = "Content-Type: text/plain\r\n";
    contentLength = "Content-Length:" + std::to_string(3) + space;
    return startLine + contentType + contentLength + "Connection: close\r\n" +
           "\r\n" + "err";
  }

  contentType = "Content-Type: text/plain\r\n";
  contentLength = "Content-Length:" + std::to_string(body.length()) + space;

  return startLine + contentType + contentLength + "\r\n" + body;
}
