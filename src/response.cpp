#include "response.hpp"
std::string Response::respond(const Request &request) const {

  std::string out;
  std::string body;
  std::string startLine;
  RequestLine requestLine = request.getRequestLine();
  std::string requestTarget = requestLine.requestTarget;
  std::string textToEcho;
  std::string space = "\r\n";
  startLine = "HTTP/1.1 200 OK\r\n";
  auto idx = requestTarget.find("/echo/", 0);
  if (requestLine.requestTarget == "/") {
    return startLine + space;
  }

  if (idx != std::string::npos) {
    requestTarget.erase(requestTarget.begin() + idx, requestTarget.begin() + 6);
    body = requestTarget;
  } else if (requestLine.requestTarget == "/user-agent") {
    body = request.getHeaderHash()["User-Agent"];
  } else {
    return "HTTP/1.1 404 NOT FOUND\r\n\r\n";
  }

  std::string contentType = "Content-Type: text/plain\r\n";
  std::string contentLength =
      "Content-Length:" + std::to_string(body.length()) + space;

  return startLine + contentType + contentLength + "\r\n" + body;
}
