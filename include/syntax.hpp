/*
 * ABNF Syntax Summary:
 *
 * 1. Rule Definition:
 *    <rule name> = <elements>
 *
 * 2. Elements:
 *    - Sequences: "A B C" (elements in order)
 *    - Alternatives: "A / B / C" (one of the elements)
 *    - Optionals: "[A]" (A is optional)
 *    - Repetitions: "A*B" (A repeated 0 to B times)
 *    - Grouping: "(A B)" (group elements)
 *
 * 3. Literals:
 *    - Case-insensitive strings: "text"
 *    - Case-sensitive strings: %s"TEXT"
 *    - Numeric values: %d123 (decimal), %x1F (hex)
 *
 * 4. Special Rules:
 *    - SP (space): %x20
 *    - CR (carriage return): %x0D
 *    - LF (line feed): %x0A
 *    - CRLF (carriage return + line feed): %x0D.0A
 *
 * 5. Comments:
 *    - Use semicolon: ; This is a comment
 *
 * Reference: https://www.rfc-editor.org/rfc/rfc5234
 */