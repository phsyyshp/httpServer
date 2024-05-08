#pragma once
#include "request.hpp"
#include "response.hpp"
class Getter {
public:
  Getter();
  void get(const RequestLine &, const std::string &dir) const;
};