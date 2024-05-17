#pragma once
#include "request.hpp"
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <unordered_map>
struct ContentMetaData {
  std::string type;
  std::string encoding;
  std::string language;
  std::string length;
  std::string location;
};
extern std::unordered_map<std::string, std::string> FILE_TYPE_MAP;
std::string getType(const std::string &path);
std::string compressBody(const std::string &toCompress);
class Response {

public:
  Response() {}
  std::string respond(const Request &request,
                      const std::string &dir = "") const;
  std::string get(const Request &request, const std::string &dir = "") const;
  std::string post(const Request &request, const std::string &dir = "") const;

  void contentNegotiation(const Request &request);
  std::string statusLine(const Request &request, int statusCode) const;
  std::string contentHeaders(const ContentMetaData &contentMetaData) const;
  std::string badRequest(const Request &) const;

  std::string echo(const Request &request) const;
  std::string userAgent(const Request &request) const;
  std::string file(const Request &request, const std::string &path) const;

  std::string notFound(const Request &request) const;

  std::string OK(const Request &request) const;
  void routeRequest() const;
};