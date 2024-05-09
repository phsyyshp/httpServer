#pragma once
#include "request.hpp"
class Response {

public:
  Response() {}
  std::string respond(const Request &request,
                      const std::string &dir = "") const;
  std::string get(const Request &request, const std::string &dir = "") const;
  std::string post(const Request &request, const std::string &dir = "") const;

  std::string statusLine(const Request &request, int statusCode) const;
  std::string contentHeaders(const std::string contentType,
                             int contentLength) const;
  std::string badRequest() const;
};