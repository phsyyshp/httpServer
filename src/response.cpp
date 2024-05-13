#include "response.hpp"
#include <cstring>
#include <string>
#include <zlib.h>
std::string getType(const std::string &path) {
  auto it = std::find(path.rbegin(), path.rend(), '.');
  std::string extention(it.base() - 1, path.end());

  return (!FILE_TYPE_MAP[extention].empty()) ? FILE_TYPE_MAP[extention]
                                             : "application/octet-stream";
}
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

  const RequestLine &requestLine = request.getRequestLine();
  const std::string &requestTarget = requestLine.requestTarget;

  if (requestLine.requestTarget == "/") {
    return OK(request);
  }

  if (requestTarget.find("/echo/", 0) != std::string::npos) {
    return echo(request);
  }

  if (requestLine.requestTarget == "/user-agent") {
    return userAgent(request);
  }

  if (requestLine.requestTarget.find("/files/", 0) != std::string::npos) {
    std::string path = dir + "/" + requestTarget.substr(7);
    return file(request, path);
  }

  if (requestLine.requestTarget[0] == '/') {
    std::string path = dir + "/" + requestTarget.substr(1);
    return file(request, path);
  }

  return notFound(request);
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
void Response::contentNegotiation(const Request &request) {
  const auto &headers = request.getHeaderHash();
};
std::string Response::echo(const Request &request) const {

  const std::string &requestTarget = request.getRequestLine().requestTarget;
  std::string body = requestTarget.substr(6);
  ContentMetaData cmd;
  cmd.length = std::to_string(body.length());
  cmd.type = "text/plain";
  if (request.getHeaderHash()["Accept-Encoding"].find("gzip") !=
      std::string::npos) {
    cmd.encoding = "gzip";
    body = compressBody(body);
    cmd.length = std::to_string(body.length()); // Set the content length
  }
  return statusLine(request, 200) + contentHeaders(cmd) + "\r\n" + body;
}
std::string Response::userAgent(const Request &request) const {
  ContentMetaData cmd;
  cmd.length = std::to_string(request.getHeaderHash()["User-Agent"].length());
  cmd.type = "text/plain";
  return statusLine(request, 200) + contentHeaders(cmd) + "\r\n" +
         request.getHeaderHash()["User-Agent"];
}
std::string Response::file(const Request &request,
                           const std::string &path) const {

  std::ifstream file;
  file.open(path);

  if (!file) {
    ContentMetaData cmd;
    cmd.length = "0";
    cmd.type = "text/plain";
    return statusLine(request, 404) + contentHeaders(cmd) + "\r\n";
  }

  std::stringstream tempBuffer;
  tempBuffer << file.rdbuf();
  std::string body = tempBuffer.str();

  ContentMetaData cmd;
  cmd.length = std::to_string(body.length());
  cmd.type = getType(path);
  return statusLine(request, 200) + contentHeaders(cmd) + "\r\n" + body;
}
std::string Response::badRequest(const Request &request) const {
  ContentMetaData cmd;
  cmd.length = "0";
  cmd.type = "text/plain";
  return statusLine(request, 400) + contentHeaders(cmd) + "\r\n";
}
std::string Response::notFound(const Request &request) const {

  ContentMetaData cmd;
  cmd.length = "0";
  cmd.type = "text/plain";
  return statusLine(request, 404) + contentHeaders(cmd) + "\r\n";
}
std::string Response::OK(const Request &request) const {
  ContentMetaData cmd;
  cmd.length = "0";
  cmd.type = "text/plain";
  return statusLine(request, 200) + contentHeaders(cmd) + "\r\n";
}
std::string compressBody(const std::string &toCompress) {

  // Initialize zlib structures
  z_stream zs; // z_stream is zlib's control structure
  memset(&zs, 0, sizeof(zs));

  if (deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, 8,
                   Z_DEFAULT_STRATEGY) != Z_OK) {
    throw(std::runtime_error("Failed to initialize zlib."));
  }

  zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(toCompress.data()));
  zs.avail_in = toCompress.size(); // Set the z_stream's input

  int ret;
  char outbuffer[32768];
  std::vector<char> compressedData;

  // Compress the data into the buffer
  do {
    zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
    zs.avail_out = sizeof(outbuffer);

    ret = deflate(&zs, Z_FINISH); // Finish compression

    if (compressedData.size() < zs.total_out) {
      compressedData.insert(compressedData.end(), outbuffer,
                            outbuffer + zs.total_out - compressedData.size());
    }
  } while (ret == Z_OK);

  deflateEnd(&zs);

  if (ret != Z_STREAM_END) {
    std::ostringstream oss;
    oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
    throw(std::runtime_error(oss.str()));
  }

  // Convert the compressed data into a std::string
  std::string body(compressedData.begin(), compressedData.end());
  return body;
}