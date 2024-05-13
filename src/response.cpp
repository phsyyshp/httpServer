#include "response.hpp"
#include <cstring>
#include <string>
#include <zlib.h>
std::string getType(const std::string &path) {
  auto it = std::find(path.rbegin(), path.rend(), '.');
  std::string extention(it.base() - 1, path.end());

  return (!FILE_TYPE_MAP[extention].empty()) ? FILE_TYPE_MAP[extention]
                                             : "text/plain";
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

  std::string contentHeader;
  RequestLine requestLine = request.getRequestLine();
  std::string requestTarget = requestLine.requestTarget;

  if (requestLine.requestTarget == "/") {
    ContentMetaData cmd;
    cmd.length = "0";
    cmd.type = "text/plain";
    return statusLine(request, 200) + contentHeaders(cmd) + "\r\n";
  }
  std::string body;
  if (requestTarget.find("/echo/", 0) != std::string::npos) {
    requestTarget.erase(requestTarget.begin(), requestTarget.begin() + 6);
    ContentMetaData cmd;

    cmd.length = std::to_string(requestTarget.length());
    cmd.type = "text/plain";
    std::cout << request.getHeaderHash()["Accept-Encoding"];
    if (request.getHeaderHash()["Accept-Encoding"].find("gzip") !=
        std::string::npos) {
      cmd.encoding = "gzip";

      // Initialize zlib structures
      z_stream zs; // z_stream is zlib's control structure
      memset(&zs, 0, sizeof(zs));

      if (deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, 8,
                       Z_DEFAULT_STRATEGY) != Z_OK) {
        throw(std::runtime_error("Failed to initialize zlib."));
      }

      zs.next_in =
          reinterpret_cast<Bytef *>(const_cast<char *>(requestTarget.data()));
      zs.avail_in = requestTarget.size(); // Set the z_stream's input

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
                                outbuffer + zs.total_out -
                                    compressedData.size());
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

      cmd.length = std::to_string(body.length()); // Set the content length

      // Construct the HTTP response
      return statusLine(request, 200) + contentHeaders(cmd) + "\r\n" + body;
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
  if (requestLine.requestTarget[0] == '/') {
    std::ifstream file;
    file.open(dir + "/" + requestTarget.substr(1));
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
    cmd.type = getType(dir + "/" + requestTarget.substr(1));
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