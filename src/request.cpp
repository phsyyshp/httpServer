#include "request.hpp"
#include <cctype>
#include <stdexcept>
#include <string>
#include <unordered_map>

// getters

RequestLine Request::getRequestLine() const { return requestLine; }
std::unordered_map<std::string, std::string> Request::getHeaderHash() const {
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
bool Request::parse(std::array<char, 1024> &buffer) {
  bool isValid = true; // isNotBadRequest, isGoodRequest
  // Step 1.
  preProcess(buffer);

  // Step 2. Request-Line
  /*
  RFC 9112: Although the request-line grammar rule requires that each of the
  component elements be separated by a single SP octet, recipients MAY instead
  parse on whitespace-delimited word boundaries and, aside from the CRLF
  terminator, treat any form of whitespace as the SP separator while ignoring
  preceding or trailing whitespace such whitespace includes one or more of the
  following octets: SP, HTAB, VT (%x0B), FF (%x0C), or bare CR.;
  */
  auto requestLineEndIt = std::find(buffer.begin(), buffer.end(), '\r');
  std::replace_if(buffer.begin(), requestLineEndIt, isWhiteSpace, ' ');
  if (!parseRequestLine(buffer.begin(), requestLineEndIt)) {
    return false;
  }

  /* RFC 9112: A recipient that receives whitespace between the start-line and
   * the first header field MUST either reject the message as invalid or ...*/
  if (isOWS(*(requestLineEndIt + 2))) {
    throw std::runtime_error("A sender MUST NOT send whitespace between the "
                             "start-line and the first header field.");
  }

  // Step 3. Headers
  auto headerLineStartIT = requestLineEndIt + 2;

  if ((*headerLineStartIT) == '\r') {
    return true;
  }
  auto headerLineEndIT = std::find(headerLineStartIT, buffer.end(), '\r');
  while (true) {
    if (*(headerLineEndIT + 2) == '\r') {
      break;
    }
    auto fieldNameEndIt = std::find(headerLineStartIT, headerLineEndIT, ':');
    if (isOWS(*(fieldNameEndIt - 1))) {
      // TODO(): implement error
      /*RFC 9112: No whitespace is allowed between the field name and colon.*/
      return false;
    }
    std::string fieldName(headerLineStartIT, fieldNameEndIt);
    /*RFC 9112: A field line value might be preceded and/or followed by optional
     * whitespace (OWS) */

    auto fieldValueStartIt = (fieldNameEndIt + 1);
    for (; fieldValueStartIt != headerLineEndIT; fieldValueStartIt++) {
      if (!isOWS(*fieldValueStartIt)) {
        break;
      }
    }

    auto fieldValueEndIt = headerLineEndIT;
    for (; fieldValueStartIt != fieldValueEndIt; fieldValueEndIt--) {
      if (!isOWS(*(fieldValueEndIt - 1))) {
        break;
      }
    }
    std::string fieldValue(fieldValueStartIt, fieldValueEndIt);
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
  /*
  RFC 9112: A server MUST respond with a 400 (Bad Request) status code to any
  HTTP/1.1 request message that lacks a Host header field and to any request
  message that contains more than one Host header field line or a Host header
  field with an invalid field value.
  */
  // TODO(): add field value and repetitions error checks
  if (headersHash["Host"].empty()) {
    return false;
  }

  return isValid;
}

void Request::skipPrecedingSP(
    std::array<char, 1024>::const_iterator &it,
    const std::array<char, 1024>::const_iterator &end) const {

  for (; it != end; it++) {
    if (*it != ' ') {
      break;
    }
  }
}
void Request::extractToken(std::array<char, 1024>::const_iterator &start,
                           const std::array<char, 1024>::const_iterator &end,
                           std::string &token) const {
  skipPrecedingSP(start, end);
  /*RFC 9112: request-line   = method SP request-target SP HTTP-version*/
  auto tokenEnd = std::find(start, end, ' ');
  token.assign(start, tokenEnd);
}
bool Request::parseRequestLine(
    const std::array<char, 1024>::const_iterator &lineStart,
    const std::array<char, 1024>::const_iterator &lineEnd) {

  auto tokenStart = lineStart;
  std::string method;
  std::string requestTarget;
  std::string version;
  extractToken(tokenStart, lineEnd, method);
  extractToken(tokenStart, lineEnd, requestTarget);

  if (!isRequestTargetValid(requestTarget)) {
    return false;
  }

  extractToken(tokenStart, lineEnd, version);
  /*
  RFC 9112:
  Recipients of an invalid request-line SHOULD respond with either a 400 (Bad
  Request) error or a 301 (Moved Permanently) redirect with the request-target
  properly encoded.
  No whitespace is allowed in the request-target.
  */
  auto versionEndIt = std::find(tokenStart, lineEnd, ' ');
  for (auto it = versionEndIt + 1; it != lineEnd; it++) {
    if (!isspace(*it)) {
      return false;
    }
  }
  requestLine.method = method;
  requestLine.requestTarget = requestTarget;
  requestLine.version = version;
  return true;
}

bool Request::isRequestTargetValid(const std::string &requestTarget) const {
  /*
  RFC:9122
  request-target = origin-form
  RFC: 9110, 9112, 3986
  origin-form    = absolute-path [ "?" query ]
  absolute-path = 1*( "/" segment )
  segment       = *pchar
  query       = *( pchar / "/" / "?" )
  */
  if (*requestTarget.begin() != '/') {
    return false;
  }
  auto absolutePathEndIt =
      std::find(requestTarget.begin(), requestTarget.end(), '?');
  if (std::find_if(requestTarget.begin() + 1, absolutePathEndIt, isPchar) !=
      absolutePathEndIt) {
    return false;
  }
  // TODO: parse Query
}
bool isPchar(char c) {
  // RFC: 3986
  // pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
  return isUnreserved(c) || isPctEncoded(c) || isSubDelims(c) || (c == ':') ||
         (c == '@');
}
bool isUnreserved(char c) {
  // RFC: 3986
  // unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"
  return std::isalpha(c) || std::isdigit(c) || (c == '-') || (c == '.') ||
         (c == '_') || (c == '~');
}
// FIX IT: HEXDIG
bool isPctEncoded(char c) {
  // RFC: 3986
  // pct-encoded = "%" HEXDIG HEXDIG
  return c == '%';
}
bool isSubDelims(char c) {
  // RFC: 3986
  // sub-delims  = "!" / "$" / "&" / "'" / "(" / ")"
  // / "*" / "+" / "," / ";" / "="
  return (c == '!') || (c == '$') || (c == '&') || (c == '\'') || (c == '(') ||
         (c == ')') || (c == '*') || (c == '+') || (c == ',') || (c == ';') ||
         (c == '=');
}
bool isOWS(char c) {
  /*RFC 9110 section
  optional whitespace
  OWS = *(SP / HTAB);
  */
  return (c == ' ' || c == '\t');
}
bool isWhiteSpace(char c) {
  /*
RFC 9112: such whitespace includes one or more of the
following octets: SP, HTAB, VT (%x0B), FF (%x0C), or bare CR.;
  */
  return isspace(c) && (c != '\n') && (c != '\r');
}