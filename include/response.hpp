#pragma once
#include "request.hpp"
class Response {
public:
  Response() {}
  std::string respond(const Request &request) const;
};