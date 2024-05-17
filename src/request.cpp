#include "request.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <iterator>
#include <mutex>
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
  /*RFC:9112
  An HTTP/1.1 message consists of a start-line followed by a CRLF and a sequence
  of octets in a format similar to the Internet Message Format [RFC5322]: zero
  or more header field lines (collectively referred to as the "headers" or the
  "header section"), an empty line indicating the end of the header section, and
  an optional message body.

  HTTP-message   = start-line CRLF
                   *( field-line CRLF )
                   CRLF
                   [ message-body ]

  for syntax notation SEE syntax.hpp Elements
  */
  if (buffer[0] == '\0') {
    std::cout << 'a';
    return false;
  }
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
  auto requestLineEnd = std::find(buffer.begin(), buffer.end(), '\r');
  std::replace_if(buffer.begin(), requestLineEnd, isWhiteSpace, ' ');
  if (!parseRequestLine(buffer.begin(), requestLineEnd)) {
    std::cout << 0;
    return false;
  }
  if (requestLineEnd == buffer.end()) {
    std::cout << 'b';
    return false;
  }

  // Step 3. Headers
  if (std::distance(requestLineEnd, buffer.end()) < 2) {
    /*RFC 9112: We must have header field lines. A server MUST respond with a
      400 (Bad Request) status code to any HTTP/1.1 request message that lacks a
      Host header field and to any request message that contains.*/
    return false;
  }
  /* RFC 9112: A recipient that receives whitespace between the start-line and
   * the first header field MUST either reject the message as invalid or ...*/
  if (isOWS(*std::next(requestLineEnd, 2))) {
    throw std::runtime_error("A sender MUST NOT send whitespace between the "
                             "start-line and the first header field.");
  }
  auto fieldLineStart = std::next(requestLineEnd, 2);

  // if ((*headerLineStartIT) == '\r') {
  //   return true;
  // }
  while (true) {
    auto fieldLineEnd = std::find(fieldLineStart, buffer.end(), '\r');
    if (fieldLineEnd == buffer.end()) {
      // field-line must have CRLF
      return false;
    }
    if (fieldLineStart == fieldLineEnd) {
      fieldLineStart = std::next(fieldLineEnd, 2);
      break;
    }
    // RFC 9112: field-line   = field-name ":" OWS field-value OWS
    auto fieldNameEnd = std::find(fieldLineStart, fieldLineEnd, ':');
    if (fieldNameEnd == fieldLineEnd) {
      // No field Name
      return false;
    }
    std::string fieldName(fieldLineStart, fieldNameEnd);
    if (fieldName.length() == 0) {
      // empty  field name.
      return false;
    }
    if (isOWS(fieldName.back())) {
      // TODO(): implement error
      /*RFC 9112: No whitespace is allowed between the field name and colon.*/

      std::cout << 17;
      return false;
    }

    /*RFC 9112: A field line value might be preceded and/or followed by optional
     * whitespace (OWS) */

    auto fieldValueStart =
        std::find_if_not(std::next(fieldNameEnd, 1), fieldLineEnd, &isOWS);
    if (fieldValueStart == fieldLineEnd) {
      // No field value;
      return false;
    }

    auto fieldValueEnd = fieldLineEnd;
    while (std::next(fieldValueEnd, -1) != fieldValueStart) {
      if (!isOWS(*std::next(fieldValueEnd, -1))) {
        break;
      }
      std::advance(fieldValueEnd, -1);
    }
    std::string fieldValue(fieldValueStart, fieldValueEnd);
    headersHash[fieldName] = fieldValue;
    fieldLineStart = fieldLineEnd + 2;
  }
  // Step 4. Body
  std::copy(fieldLineStart, buffer.end(), body.begin());
  /*
  RFC 9112: A server MUST respond with a 400 (Bad Request) status code to any
  HTTP/1.1 request message that lacks a Host header field and to any request
  message that contains more than one Host header field line or a Host header
  field with an invalid field value.
  */
  // TODO(): add field value and repetitions error checks
  if (headersHash["Host"].empty()) {
    std::cout << 11;
    return false;
  }

  return isValid;
}

void Request::skipPrecedingSP(
    std::array<char, 1024>::const_iterator &it,
    const std::array<char, 1024>::const_iterator &end) const {
  it = std::find_if_not(it, end, [](char c) { return c == ' '; });
}
bool Request::extractToken(std::array<char, 1024>::const_iterator &start,
                           const std::array<char, 1024>::const_iterator &end,
                           std::string &token) const {
  skipPrecedingSP(start, end);
  /*RFC 9112: request-line   = method SP request-target SP HTTP-version*/
  if (start == end) {
    // empty token
    return false;
  }
  auto tokenEnd = std::find(start, end, ' ');
  if (tokenEnd == end) {
    // only one token
    return false;
  }

  token.assign(start, tokenEnd);
  start = std::next(tokenEnd, 1);
  return true;
}
bool Request::parseRequestLine(
    const std::array<char, 1024>::const_iterator &lineStart,
    const std::array<char, 1024>::const_iterator &lineEnd) {
  /*RFC: 9112
   request-line   = method SP request-target SP HTTP-version
  */
  // std::cout << *lineStart;
  // std::cout.flush();
  auto tokenStart = lineStart;
  std::string method;
  std::string requestTarget;
  std::string version;
  if (!extractToken(tokenStart, lineEnd, method)) {
    return false;
  }
  if (!extractToken(tokenStart, lineEnd, requestTarget)) {
    return false;
  }

  if (!extractAbsolutePath(requestTarget)) {

    std::cout << 1;
    return false;
  }
  // We can't use extract token here because it will modify the iterator of
  // start of version token, which is needed.
  skipPrecedingSP(tokenStart, lineEnd);
  if (tokenStart == lineEnd) {
    return false;
  }
  auto versionEnd = std::find(tokenStart, lineEnd, ' ');
  version.assign(tokenStart, versionEnd);
  /*
  RFC 9112:
  Recipients of an invalid request-line SHOULD respond with either a 400..
  No whitespace is allowed in the request-target.
  */

  // FIX IT: this is not robust
  if (versionEnd != lineEnd) {

    auto nonSPChar =
        std::find_if_not(versionEnd, lineEnd, [](char c) { return c == ' '; });
    if (nonSPChar != lineEnd) {
      // more than 3 tokens in request-line
      return false;
    }
  }
  requestLine.method = method;
  requestLine.requestTarget = requestTarget;
  requestLine.version = version;
  return true;
}

bool Request::extractAbsolutePath(std::string &requestTarget) const {
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
    std::cout << 3;
    return false;
  }
  auto absolutePathEndIt =
      std::find(requestTarget.begin(), requestTarget.end(), '?');
  // if (std::find_if(requestTarget.begin() + 1, absolutePathEndIt, isNotPchar)
  // !=
  //     absolutePathEndIt) {
  //   std::cout << *std::find_if(requestTarget.begin() + 1, absolutePathEndIt,
  //                              isNotPchar);
  //   std::cout << 4;
  //   return false;
  // }
  // TODO: parse Query
  requestTarget.assign(requestTarget.begin(), absolutePathEndIt);
  return true;
}

bool isNotPchar(char c) {
  // RFC: 3986
  // pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
  return !isPchar(c);
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