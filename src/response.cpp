#include "response.hpp"
#include <string>
std::string Response::respond(const Request &request,
                              const std::string &dir) const {

  RequestLine requestLine = request.getRequestLine();
  if (requestLine.method == "GET") {
    return get(request, dir);
  }
  if (requestLine.method == "POST") {
    return post(request, dir);
  }
}

std::string Response::post(const Request &request,
                           const std::string &dir) const {

  RequestLine requestLine = request.getRequestLine();
  std::string requestTarget = requestLine.requestTarget;

  if (requestLine.requestTarget.find("/files/", 0) != std::string::npos) {
    std::ofstream file;
    file.open(dir + "/" + requestTarget.substr(7));
    if (!file) {
      return statusLine(request, 404) + contentHeaders("text/plain", 0) +
             "\r\n";
    }
    file << request.getBody().data();
    return statusLine(request, 201) +
           contentHeaders("Content-Type: application/octet-stream", 0) + "\r\n";
  }
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
    std::ifstream file;
    file.open(dir + "/" + requestTarget.substr(7));
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
  }
  return statusLine(request, 404) + contentHeaders("text/plain", 0) + "\r\n";
}

std::string Response::statusLine(const Request &request, int statusCode) const {
  std::string reasonPhrase;
  switch (statusCode) {
  case 200:
    reasonPhrase = "OK";
    break;
  case 201:
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