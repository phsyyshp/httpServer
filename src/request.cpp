#include "request.hpp"
#include <string>
#include <unordered_map>

RequestLine Request::getRequestLine() const {
  RequestLine requestLine;
  std::string tempStr;
  for (int i = 0; i < buffer.size() - 1; i++) {
    char letter = buffer[i];
    if (letter == '\r' && buffer[i + 1] == '\n') {
      break;
    }
    tempStr.push_back(letter);
  }
  auto tokens = tokenize(tempStr);
  int tokenNO = 0;
  for (const auto &token : tokens) {
    switch (tokenNO) {
    case METHOD:
      requestLine.method = token;
      break;
    case REQUEST_TARGET:
      requestLine.requestTarget = token;
      break;
    case VERSION:
      requestLine.version = token;
      break;
    default:
      break;
    }
    tokenNO++;
  }
  return requestLine;
}

std::array<char, 1024> Request::getBody() const {

  std::string requestLine;
  int i = 0;
  for (i = 0; i < buffer.size() - 1; i++) {
    char letter = buffer[i];
    if (letter == '\r' && buffer[i + 1] == '\n') {
      break;
    }
    requestLine.push_back(letter);
  }

  // std::string headerFieldLines;
  std::unordered_map<std::string, std::string> headersHash;
  int idxOfTokenOnLine = 0;
  int lineNumber = 0;
  std::string token;
  std::string fieldName;
  std::string fieldValue;

  int headerLinesStartIdx = i + 2;
  if (buffer[headerLinesStartIdx] == '\r' &&
      buffer[headerLinesStartIdx + 1] == '\n') {
    ;
  } else {
    // Header field lines
    for (i = headerLinesStartIdx; i < buffer.size() - 3; i++) {
      char letter = buffer[i];
      if (letter == '\r' && buffer[i + 1] == '\n' && buffer[i + 2] == '\r' &&
          buffer[i + 3] == '\n') {
        break;
      }
      if (letter == '\r' && buffer[i + 1] == '\n') {
        idxOfTokenOnLine = 0;
        fieldName.pop_back(); // delete the :;
        headersHash[fieldName] = fieldValue;
        fieldName = "";
        fieldValue = "";

        i++; // In order to skip next '\n'
        continue;
      }

      if (letter == ' ') {
        idxOfTokenOnLine++;
        continue;
      }
      if (idxOfTokenOnLine == 0) {
        fieldName.push_back(letter);
      } else {
        fieldValue.push_back(letter);
      }
    }
  }
  int bodyLineStartIdx = i + 4;
  std::array<char, 1024> body;
  int j = 0;
  for (int i = bodyLineStartIdx; i < buffer.size(); i++) {
    body[j] = buffer[i];
    j++;
  }
  return body;
}

std::unordered_map<std::string, std::string> Request::getHeaderHash() const {

  std::string requestLine;
  int i = 0;
  for (i = 0; i < buffer.size() - 1; i++) {
    char letter = buffer[i];
    if (letter == '\r' && buffer[i + 1] == '\n') {
      break;
    }
    requestLine.push_back(letter);
  }

  // std::string headerFieldLines;
  std::unordered_map<std::string, std::string> headersHash;
  int idxOfTokenOnLine = 0;
  int lineNumber = 0;
  std::string token;
  std::string fieldName;
  std::string fieldValue;

  int headerLinesStartIdx = i + 2;
  if (buffer[headerLinesStartIdx] == '\r' &&
      buffer[headerLinesStartIdx + 1] == '\n') {
    ;
  } else {
    // Header field lines
    for (int i = headerLinesStartIdx; i < buffer.size() - 3; i++) {
      char letter = buffer[i];
      if (letter == '\r' && buffer[i + 1] == '\n' && buffer[i + 2] == '\r' &&
          buffer[i + 3] == '\n') {
        break;
      }
      if (letter == '\r' && buffer[i + 1] == '\n') {
        idxOfTokenOnLine = 0;
        fieldName.pop_back(); // delete the :;
        headersHash[fieldName] = fieldValue;
        fieldName = "";
        fieldValue = "";

        i++; // In order to skip next '\n'
        continue;
      }

      if (letter == ' ') {
        idxOfTokenOnLine++;
        continue;
      }
      if (idxOfTokenOnLine == 0) {
        fieldName.push_back(letter);
      } else {
        fieldValue.push_back(letter);
      }
    }
  }
  return headersHash;
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
