#pragma once
#include "request.hpp"
class Response {

public:
  Response() {}
  std::string respond(const Request &request,
                      const std::string &dir = "") const;
  std::string get(const Request &request,
                      const std::string &dir = "") const;
  std::string post(const Request &request,
                      const std::string &dir = "") const;
                       
                       
};