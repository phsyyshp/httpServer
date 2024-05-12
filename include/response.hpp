#pragma once
#include "request.hpp"
#include <cstdlib>
#include <memory>
struct ContentMetaData {
  std::string type;
  std::string encoding;
  std::string language;
  std::string length;
  std::string location;
};
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
};