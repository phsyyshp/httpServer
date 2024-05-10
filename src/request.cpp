#include "request.hpp"
#include <stdexcept>
#include <string>
#include <unordered_map>

// getters

RequestLine Request::getRequestLine() const { return requestLine; }
std::unordered_map<std::string, std::string> Request::getHeaderHash() const {
  for (auto [key, value] : headersHash) {

    std::cout << key << " " << value << "\n";
  }
  return headersHash;
}
std::array<char, 1024> Request::getBody() const { return body; }
void Request::preProcess(std::array<char, 1024> &buffer) {
  /* RFC 9112: A recipient of such a bare CR MUST consider that element to be
  invalid or replace each bare CR with SP before processing the element or
  forwarding the message.*/
  for (int i = 0; i < buffer.size() - 1; i++) {
    if ((buffer[i] == '\r') && (buffer[i + 1] != '\n')) {
      buffer[i] = ' ';
    }
  }
}
void Request::parse(std::array<char, 1024> &buffer) {

  // Step 1.
  preProcess(buffer);

  // Step 2. Request-Line

  auto requestLineEndIt = std::find(buffer.begin(), buffer.end(), '\r');
  /* RFC 9112: A recipient that receives whitespace between the start-line and
   * the first header field MUST either reject the message as invalid or ...*/
  if (isWhiteSpace(*(requestLineEndIt + 2))) {
    throw std::runtime_error("A sender MUST NOT send whitespace between the "
                             "start-line and the first header field.");
  }
  auto methodEndIt = std::find(buffer.begin(), requestLineEndIt, ' ');
  auto requestTargetIt = std::find(methodEndIt + 1, requestLineEndIt, ' ');

  std::string method(buffer.begin(), methodEndIt);
  std::string requestTarget(methodEndIt + 1, requestTargetIt);
  std::string version(requestTargetIt + 1, requestLineEndIt);

  requestLine.method = method;
  requestLine.requestTarget = requestTarget;
  requestLine.version = version;

  // Step 3. Headers

  std::string fieldValue;
  auto headerLineStartIT = requestLineEndIt + 2;

  if ((*headerLineStartIT) == '\r') {
    return;
  }
  auto headerLineEndIT = std::find(headerLineStartIT, buffer.end(), '\r');
  while (true) {
    if (*(headerLineEndIT + 2) == '\r') {
      break;
    }
    auto fieldNameEndIt = std::find(headerLineStartIT, headerLineEndIT, ':');
    if (isWhiteSpace(*(fieldNameEndIt - 1))) {
      // TODO(): implement error
      /*RFC 9112: No whitespace is allowed between the field name and colon.*/
      break;
    }
    std::string fieldName(headerLineStartIT, fieldNameEndIt);
    /*RFC 9112: A field line value might be preceded and/or followed by optional
     * whitespace (OWS) */

    auto fieldValueStartIt = (fieldNameEndIt + 1);
    for (; fieldValueStartIt != headerLineEndIT; fieldValueStartIt++) {
      if (!isWhiteSpace(*fieldValueStartIt)) {
        break;
      }
    }

    auto fieldValueEndIt = headerLineEndIT;
    for (; fieldValueStartIt != fieldValueEndIt; fieldValueEndIt--) {
      if (!isWhiteSpace(*(fieldValueEndIt - 1))) {
        break;
      }
    }
    headersHash[fieldName] = fieldValue;
    headerLineStartIT = headerLineEndIT + 2;
    headerLineEndIT = std::find(headerLineStartIT, buffer.end(), '\r');
  }
  // Step 4. Body
  auto bodyStartIt = headerLineEndIT + 4;
  int i = 0;
  for (auto it = bodyStartIt; buffer.end() != it; it++) {
    body[i] = *it;
    i++;
  }
}

bool Request::isWhiteSpace(char c) const {
  /*RFC 9110 section
  optional whitespace
  OWS = *(SP / HTAB);
  */
  return (c == ' ' || c == '\t');
}