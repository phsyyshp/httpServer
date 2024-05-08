#include "request.hpp"
#include <string>
#include <unordered_map>

std::string Request::getRequestLine() const {
  std::string out;
  for (int i = 0; i < buffer.size() - 1; i++) {
    char letter = buffer[i];
    if (letter == '\r' && buffer[i + 1] == '\n') {
      break;
    }
    out.push_back(letter);
  }
  return out;
}

std::unordered_map<std::string, std::string> Request::parseRequestLine() const {

  auto tokens = tokenize(getRequestLine());
  std::unordered_map<std::string, std::string> out;
  int tokenNO = 0;
  for (const auto &token : tokens) {

    switch (tokenNO) {

    case METHOD:
      break;
    case PATH:
      out["path"] = token;
      break;
    case VERSION:
      out["version"] = token;
      break;
    default:
      break;
    }
    tokenNO++;
  }
  return out;
}
std::vector<std::string> Request::tokenize(const std::string &string) const {
  std::stringstream stream(string);
  std::string token;
  std::vector<std::string> out;
  while (stream >> token) {
    out.push_back(token);
  }
  return out;
}
