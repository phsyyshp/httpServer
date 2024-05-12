#pragma once
#include <algorithm>
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
bool isOWS(char c);
bool isWhiteSpace(char c);
bool isUnreserved(char c);
bool isPctEncoded(char c);
bool isSubDelims(char c);
bool isPchar(char c);
bool isNotPchar(char c);
class Request {
public:
  Request() = default;
  bool parse(std::array<char, 1024> &buffer);
  RequestLine getRequestLine() const;

  std::unordered_map<std::string, std::string> getHeaderHash() const;
  std::array<char, 1024> getBody() const;

private:
  bool parseRequestLine(const std::array<char, 1024>::const_iterator &lineStart,
                        const std::array<char, 1024>::const_iterator &lineEnd);
  void extractToken(std::array<char, 1024>::const_iterator &start,
                    const std::array<char, 1024>::const_iterator &end,
                    std::string &token) const;
  bool isRequestTargetValid(const std::string &) const;
  void preProcess(std::array<char, 1024> &buffer);
  void skipPrecedingSP(std::array<char, 1024>::const_iterator &it,
                       const std::array<char, 1024>::const_iterator &end) const;
  RequestLine requestLine;
  std::unordered_map<std::string, std::string> headersHash;
  std::array<char, 1024> body;
};