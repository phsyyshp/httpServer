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
  Request();
  void preProcess(std::array<char, 1024> &buffer);
  void parse(std::array<char, 1024> &buffer);
  bool isWhiteSpace(char) const;
  RequestLine getRequestLine() const;
  std::unordered_map<std::string, std::string> getHeaderHash() const;
  std::array<char, 1024> getBody() const;

private:
  RequestLine requestLine;

  std::unordered_map<std::string, std::string> headersHash;
  std::array<char, 1024> body;
};