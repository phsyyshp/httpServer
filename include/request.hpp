#pragma once
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
enum requestToken { METHOD, REQUEST_TARGET, VERSION };
struct RequestLine {
  std::string method;
  std::string requestTarget;
  std::string version;
};
class Request {
public:
  Request(const std::array<char, 1024> &buffer_) : buffer(buffer_) {}
  RequestLine getRequestLine() const;
  std::unordered_map<std::string, std::string> getHeaderHash() const;
  std::string getBody() const;

private:
  std::array<char, 1024> buffer;
  std::vector<std::string> tokenize(const std::string &) const;
};