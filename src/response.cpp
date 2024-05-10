#include "response.hpp"
#include <string>
std::string Response::respond(const Request &request,
                              const std::string &dir) const {

  RequestLine requestLine = request.getRequestLine();
  if (requestLine.method == "GET") {
    return get(request, dir);
  } else if (requestLine.method == "POST") {
    return post(request, dir);
  }
  return "a";
}

std::string Response::post(const Request &request,
                           const std::string &dir) const {

  std::string out;
  std::string body;
  std::string startLine;
  std::string contentLength;
  std::string contentType;
  RequestLine requestLine = request.getRequestLine();
  std::string requestTarget = requestLine.requestTarget;
  std::string space = "\r\n";
  auto idx2 = requestLine.requestTarget.find("/files/", 0);
  std::string fileName;

  if (idx2 != std::string::npos) {
    requestTarget.erase(requestTarget.begin() + idx2,
                        requestTarget.begin() + 7);
    fileName = requestTarget;
    std::ofstream file;
    file.open(dir + "/" + fileName);
    // std::cout << request.getBody();
    if (!file) {
      startLine = "HTTP/1.1 404 NOT FOUND\r\n";
      contentType = "Content-Type: text/plain\r\n";
      contentLength = "Content-Length:" + std::to_string(3) + space;
      return startLine + contentType + contentLength + "Connection: close\r\n" +
             "\r\n" + "err";
    }

    startLine = "HTTP/1.1 201 OK\r\n";
    contentType = "Content-Type: application/octet-stream\r\n";
    // std::cout << request.getBody();
    file << request.getBody().data();
    contentLength = "Content-Length:" + std::to_string(7) + space;
    return startLine + contentType + contentLength + "\r\n" + "written";
  }
  return "A";
}
std::string Response::get(const Request &request,
                          const std::string &dir) const {

  std::string body;
  std::string contentHeader;
  RequestLine requestLine = request.getRequestLine();
  std::string requestTarget = requestLine.requestTarget;

  if (requestLine.requestTarget == "/") {
    return statusLine(request, 200) + contentHeaders("text/plain", 0) + "\r\n";
  }

  if (requestTarget.find("/echo/", 0) != std::string::npos) {
    requestTarget.erase(requestTarget.begin(), requestTarget.begin() + 6);
    return statusLine(request, 200) +
           contentHeaders("text/plain", requestTarget.length()) + "\r\n" +
           requestTarget;
  }
  if (requestLine.requestTarget == "/user-agent") {
    return statusLine(request, 200) +
           contentHeaders("text/plain",
                          request.getHeaderHash()["User-Agent"].length()) +
           "\r\n" + request.getHeaderHash()["User-Agent"];
  }
  if (requestLine.requestTarget.find("/files/", 0) != std::string::npos) {
    requestTarget.erase(requestTarget.begin(), requestTarget.begin() + 7);
    std::ifstream file;
    file.open(dir + "/" + requestTarget);
    if (!file) {
      return statusLine(request, 404) + contentHeaders("text/plain", 0) +
             "\r\n";
    }
    std::string line;
    while (getline(file, line)) {
      body += line;
    }
    return statusLine(request, 200) +
           contentHeaders("application/octet-stream", body.length()) + "\r\n" +
           body;
  } else {
    return statusLine(request, 404) + contentHeaders("text/plain", 0) + "\r\n";
  }
}

std::string Response::statusLine(const Request &request, int statusCode) const {
  std::string reasonPhrase;
  switch (statusCode) {
  case 200:
    reasonPhrase = "OK";
    break;
  case 400:
    reasonPhrase = "NOT FOUND";
    break;
  default:
    break;
  }
  return request.getRequestLine().version + " " + std::to_string(statusCode) +
         " " + reasonPhrase + "\r\n";
}
std::string Response::contentHeaders(const std::string contentType,
                                     int contentLength) const {

  return "Content-Type: " + contentType + "\r\n" +
         "Content-Length: " + std::to_string(contentLength) + "\r\n";
}