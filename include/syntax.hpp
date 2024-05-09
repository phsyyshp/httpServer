/*
The rules below are defined in [HTTP]:
  BWS           = <BWS, see [HTTP], Section 5.6.3>
  OWS           = <OWS, see [HTTP], Section 5.6.3>
  RWS           = <RWS, see [HTTP], Section 5.6.3>
  absolute-path = 1*( "/" segment )
  field-name    = <field-name, see [HTTP], Section 5.1>
  field-value   = <field-value, see [HTTP], Section 5.5>
  obs-text      = <obs-text, see [HTTP], Section 5.6.4>
  quoted-string = <quoted-string, see [HTTP], Section 5.6.4>
  token          = 1*tchar
  transfer-coding =
                  <transfer-coding, see [HTTP], Section 10.1.4>
  origin-form    = absolute-path [ "?" query ]
  absolute-form  = absolute-URI
  absolute-URI  = scheme ":" hier-part [ "?" query ]
  authority     = <authority, see [URI], Section 3.2>
  uri-host      = <host, see [URI], Section 3.2.2>
  port          = <port, see [URI], Section 3.2.3>
  segment       = *pchar
  pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
  unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"
  pct-encoded = "%" HEXDIG HEXDIG
  sub-delims  = "!" / "$" / "&" / "'" / "(" / ")"
                  / "*" / "+" / "," / ";" / "="
  query       = *( pchar / "/" / "?" )
  tchar          = "!" / "#" / "$" / "%" / "&" / "'" / "*"
                 / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"
                 / DIGIT / ALPHA
                 ; any VCHAR, except delimiters
hier-part   = "//" authority path-abempty
  / path-absolute
  / path-rootless
  / path-empty
path          = path-abempty    ; begins with "/" or is empty
                / path-absolute   ; begins with "/" but not "//"
                / path-noscheme   ; begins with a non-colon segment
                / path-rootless   ; begins with a segment
                / path-empty      ; zero characters
path-abempty  = *( "/" segment )
path-absolute = "/" [ segment-nz *( "/" segment ) ]
path-noscheme = segment-nz-nc *( "/" segment )
path-rootless = segment-nz *( "/" segment )
path-empty    = 0<pchar>
segment-nz    = 1*pchar
segment-nz-nc = 1*( unreserved / pct-encoded / sub-delims / "@" )
                ; non-zero-length segment without any colon ":"*/

/*Tokens are short textual identifiers that do not include whitespace or
  delimiters.
                 */

/*  URI-reference = <URI-reference, see [URI], Section 4.1>
absolute-URI  = <absolute-URI, see [URI], Section 4.3>
relative-part = <relative-part, see [URI], Section 4.2>
authority     = <authority, see [URI], Section 3.2>
uri-host      = <host, see [URI], Section 3.2.2>
port          = <port, see [URI], Section 3.2.3>
path-abempty  = <path-abempty, see [URI], Section 3.3>
partial-URI   = relative-part [ "?" query ]*/

/*
 */