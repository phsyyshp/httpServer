#include <array>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
enum requestToken { METHOD, PATH, VERSION };
class Request {
public:
  Request(const std::array<char, 1024> &buffer_) : buffer(buffer_) {}
  std::string getRequestLine() const;
  std::unordered_map<std::string, std::string> parseRequestLine() const;

private:
  std::array<char, 1024> buffer;
  std::vector<std::string> tokenize(const std::string &) const;
};