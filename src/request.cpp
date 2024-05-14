#include "request.hpp"
#include <cctype>
#include <iostream>
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
  auto requestLineEndIt = std::find(buffer.begin(), buffer.end(), '\r');
  std::replace_if(buffer.begin(), requestLineEndIt, isWhiteSpace, ' ');
  if (!parseRequestLine(buffer.begin(), requestLineEndIt)) {
    std::cout << 0;
    return false;
  }
  if (requestLineEndIt == buffer.end()) {
    return false;
  }

  /* RFC 9112: A recipient that receives whitespace between the start-line and
   * the first header field MUST either reject the message as invalid or ...*/
  if (isOWS(*(requestLineEndIt + 2))) { // Potential buffer overflow here!!!
                                        // requestLineEndIt might not be buffer.end() but it might be buffer.end() - 2
                                        // and causing buffer overflow!!
    throw std::runtime_error("A sender MUST NOT send whitespace between the "
                             "start-line and the first header field.");
  }

  // Step 3. Headers
  auto position = requestLineEndIt + 2;

  // if ((*headerLineStartIT) == '\r') {
  //   return true;
  // }
  // auto headerLineEndIT = std::find(headerLineStartIT, buffer.end(), '\r');
  // int infLooper = 0;
  // while (true) {
  //   if (infLooper == 5000) {
  //     throw std::runtime_error("infinite loop");
  //   }
  //   infLooper++;
  //   auto fieldNameEndIt = std::find(headerLineStartIT, headerLineEndIT, ':');
  //   if (isOWS(*(fieldNameEndIt - 1))) { //Here first need to check if ':' is actaully found or not by comparing fieldNameEndIt with headerLineEndIT!!
  //     // TODO(): implement error
  //     /*RFC 9112: No whitespace is allowed between the field name and colon.*/

  //     std::cout << 17;
  //     return false;
  //   }
  //   std::string fieldName(headerLineStartIT, fieldNameEndIt);
  //   /*RFC 9112: A field line value might be preceded and/or followed by optional
  //    * whitespace (OWS) */

  //   auto fieldValueStartIt = (fieldNameEndIt + 1);
  //   for (; fieldValueStartIt != headerLineEndIT; fieldValueStartIt++) {
  //     if (!isOWS(*fieldValueStartIt)) {
  //       break;
  //     }
  //   }

  //   auto fieldValueEndIt = headerLineEndIT;
  //   for (; fieldValueStartIt != fieldValueEndIt; fieldValueEndIt--) {
  //     if (!isOWS(*(fieldValueEndIt - 1))) {
  //       break;
  //     }
  //   }
  //   std::string fieldValue(fieldValueStartIt, fieldValueEndIt);
  //   headersHash[fieldName] = fieldValue;
  //   if (*(headerLineEndIT + 2) == '\r') {
  //     break;
  //   }
  //   headerLineStartIT = headerLineEndIT + 2;
  //   headerLineEndIT = std::find(headerLineStartIT, buffer.end(), '\r');
  // }
  // A preferably better implementation might be this:
  // it's always important to know that
  // the iterators are in valid range and that there isn't buffer overflows, the strategy here
  // is to have a position iterator at the start of the header line.
  // Then find the end of the header line, parse the header, validate name and value
  // Continue to the next line until the end of header is reached.
  // HTTP-message   = start-line CRLF
  //                *( field-line CRLF )----> *(ows)?fieldName:*(ows)fieldValue*(ows)CRLF
  //                CRLF
  //                [ message-body ]
  while (true) {
    auto lineEnd = std::find(position, buffer.end(), '\r');
    if (lineEnd == buffer.end()) {
      //Broken header line
      return false;
    }
    if (position == lineEnd) {
      //We hit the CRLF signaling the end of the header
      position = lineEnd + 2;
      //+2 is safe here since there is a LF following lineEnd and additional step is allowed to the reach the buffer.end() in the worst case
      break;
    }
    auto colonPosition = std::find(position, lineEnd, ':');
    if (colonPosition == lineEnd) {
      //Broken header line: No header field name was found!
      return false;
	}
	std::string fieldName(position, colonPosition);
	if (fieldName.size() == 0) {
      //Broken header line: empty header field name!
      return false;
    }
    //Now that we know fieldName is non-empty we can freely check if the last element is ows or not
    if (isOWS(fieldName.back())) {
      //Broken header line
      /*RFC 9112: No whitespace is allowed between the field name and colon.*/
      return false;
	}
    //Looks like fieldName is valid, so we can move the position one past colonPosition
    position = colonPosition + 1;
    //Now skip the potentially preceeding optional white spaces...
    auto fieldValueStart = std::find_if_not(position, lineEnd, &isOWS);
    if (fieldValueStart == lineEnd) {
      //Broken header line
      //No field value was found
      return false;
    }
    //Now skip the potentially following optional white spaces...
    auto fieldValueEnd = lineEnd;
    while (fieldValueEnd - 1 != fieldValueStart) {
      if (!isOWS(*(fieldValueEnd - 1))) {
        break;
      }
      --fieldValueEnd;
    }
    //Previously confirmed that there is at least one character
    //different than optional white space in the interval [fieldValueStart, lineEnd)
    //so it's safe to set fieldValue to be the string in the interval [fieldValueStart, fieldValueEnd) since it won't be empty
    std::string fieldValue(fieldValueStart, fieldValueEnd);
    //Save the header data
    headersHash[fieldName] = fieldValue;
    //set position to the next of LN which is following lineEnd
    position = lineEnd + 2; //CRLF
  }
  // Step 4. Body
  // auto bodyStartIt = lineEnd + 4; Now it automatically became lineEnd
  std::copy(position, buffer.end(), body.begin()); //Copy the rest to body buffer, this might be empty all we know
  // int i = 0;
  // for (auto it = bodyStartIt; buffer.end() != it; it++) {
  //   body[i] = *it;
  //   i++;
  // }
  /*
  RFC 9112: A server MUST respond with a 400 (Bad Request) status code to any
  HTTP/1.1 request message that lacks a Host header field and to any request
  message that contains more than one Host header field line or a Host header
  field with an invalid field value.
  */
  // TODO(): add field value and repetitions error checks
  if (headersHash.find("Host") == headersHash.end()) {
    std::cout << 11;
    return false;
  }

  return true;
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
bool Request::extractToken(std::array<char, 1024>::const_iterator &start,
                           const std::array<char, 1024>::const_iterator &end,
                           std::string &token) const {
  skipPrecedingSP(start, end);
  /*RFC 9112: request-line   = method SP request-target SP HTTP-version*/
  auto tokenEnd = std::find(start, end, ' ');
  if (start != tokenEnd) {

    token.assign(start, tokenEnd);
    start = tokenEnd + 1;
    return true;
  }
  return false;
  // if(tokenEnd!=end)
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
  auto tokenEnd = std::find(tokenStart, lineEnd, ' ');
  version.assign(tokenStart, tokenEnd);
  /*
  RFC 9112:
  Recipients of an invalid request-line SHOULD respond with either a 400 (Bad
  Request) error or a 301 (Moved Permanently) redirect with the request-target
  properly encoded.
  No whitespace is allowed in the request-target.
  */

  // FIX IT: this is not robust
  auto versionEndIt = std::find(tokenStart, lineEnd, ' ');
  for (auto it = versionEndIt; it != lineEnd; it++) {
    if (!isspace(*it)) {
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
    std::cout << requestTarget;
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