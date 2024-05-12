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

  ContentMetaData cmd;
  cmd.length = "0";
  cmd.type = "text/plain";
  return statusLine(request, 501) + contentHeaders(cmd) + "\r\n";
}

std::string Response::post(const Request &request,
                           const std::string &dir) const {

  RequestLine requestLine = request.getRequestLine();
  std::string requestTarget = requestLine.requestTarget;

  if (requestLine.requestTarget.find("/files/", 0) != std::string::npos) {
    std::ofstream file;
    file.open(dir + "/" + requestTarget.substr(7));
    if (!file) {

      ContentMetaData cmd;
      cmd.length = "0";
      cmd.type = "text/plain";
      return statusLine(request, 404) + contentHeaders(cmd) + "\r\n";
    }
    file << request.getBody().data();
    ContentMetaData cmd;
    cmd.length = "0";
    cmd.type = "application/octet-stream";
    return statusLine(request, 201) + contentHeaders(cmd) + "\r\n";
  }
  ContentMetaData cmd;
  cmd.length = "0";
  cmd.type = "text/plain";
  return statusLine(request, 404) + contentHeaders(cmd) + "\r\n";
}
std::string Response::get(const Request &request,
                          const std::string &dir) const {

  std::string body;
  std::string contentHeader;
  RequestLine requestLine = request.getRequestLine();
  std::string requestTarget = requestLine.requestTarget;

  if (requestLine.requestTarget == "/") {
    ContentMetaData cmd;
    cmd.length = "0";
    cmd.type = "text/plain";
    return statusLine(request, 200) + contentHeaders(cmd) + "\r\n";
  }

  if (requestTarget.find("/echo/", 0) != std::string::npos) {
    requestTarget.erase(requestTarget.begin(), requestTarget.begin() + 6);
    ContentMetaData cmd;

    cmd.length = std::to_string(requestTarget.length());
    cmd.type = "text/plain";
    std::cout << request.getHeaderHash()["Accept-Encoding"];
    if (request.getHeaderHash()["Accept-Encoding"].find("gzip") !=
        std::string::npos) {
      cmd.encoding = "gzip";
    }
    return statusLine(request, 200) + contentHeaders(cmd) + "\r\n" +
           requestTarget;
  }
  if (requestLine.requestTarget == "/user-agent") {
    ContentMetaData cmd;
    cmd.length = std::to_string(request.getHeaderHash()["User-Agent"].length());
    cmd.type = "text/plain";
    return statusLine(request, 200) + contentHeaders(cmd) + "\r\n" +
           request.getHeaderHash()["User-Agent"];
  }
  if (requestLine.requestTarget.find("/files/", 0) != std::string::npos) {
    std::ifstream file;
    file.open(dir + "/" + requestTarget.substr(7));
    if (!file) {
      ContentMetaData cmd;
      cmd.length = "0";
      cmd.type = "text/plain";
      return statusLine(request, 404) + contentHeaders(cmd) + "\r\n";
    }
    std::string line;
    while (getline(file, line)) {
      body += line;
    }
    ContentMetaData cmd;
    cmd.length = std::to_string(body.length());
    cmd.type = "application/octet-stream";
    return statusLine(request, 200) + contentHeaders(cmd) + "\r\n" + body;
  }

  ContentMetaData cmd;
  cmd.length = "0";
  cmd.type = "text/plain";
  return statusLine(request, 404) + contentHeaders(cmd) + "\r\n";
}

std::string Response::statusLine(const Request &request, int statusCode) const {
  std::string reasonPhrase;
  switch (statusCode) {
  case 200:
    reasonPhrase = "OK";
    break;
  case 201:
    reasonPhrase = "Created";
    break;
  case 400:
    reasonPhrase = "BAD REQUEST";
    break;
  case 404:
    reasonPhrase = "Not Found";
    break;
  case 501:
    reasonPhrase = "NOT IMPLEMENTED";
  default:
    break;
  }
  return request.getRequestLine().version + " " + std::to_string(statusCode) +
         " " + reasonPhrase + "\r\n";
}
std::string
Response::contentHeaders(const ContentMetaData &contentMetaData) const {
  std::string out;
  if (!contentMetaData.encoding.empty()) {
    out += "Content-Encoding: " + contentMetaData.encoding + "\r\n";
  }

  if (!contentMetaData.type.empty()) {
    out += "Content-Type: " + contentMetaData.type + "\r\n";
  }
  if (!contentMetaData.length.empty()) {
    out += "Content-Length: " + contentMetaData.length + "\r\n";
  }
  if (!contentMetaData.language.empty()) {
    out += "Content-Language: " + contentMetaData.language + "\r\n";
  }
  if (!contentMetaData.location.empty()) {
    out += "Content-Location: " + contentMetaData.location + "\r\n";
  }

  return out;
}
std::string Response::badRequest(const Request &request) const {
  ContentMetaData cmd;
  cmd.length = "0";
  cmd.type = "text/plain";
  return statusLine(request, 400) + contentHeaders(cmd) + "\r\n";
}
void Response::contentNegotiation(const Request &request) {
  const auto &headers = request.getHeaderHash();
};