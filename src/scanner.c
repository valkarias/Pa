//> Scanning on Demand scanner-c
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

typedef struct {
  const char* start;
  const char* current;
  int line;
} Scanner;

Scanner scanner;
//> init-scanner
void initScanner(const char* source) {
  scanner.start = source;
  scanner.current = source;
  scanner.line = 1;
}

static bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
          c == '_';
}

static bool isDigit(char c) {
  return (c >= '0' && c <= '9');
}

static bool isHex(char c) {
  return (
    (c >= '0' && c <= '9') || 
    (c >= 'A' && c <= 'F') || 
    (c >= 'a' && c <= 'f') || 
    (c == '_')
  );
}

static bool isOct(char c) {
  return (
    (c >= '0' && c <= '9') ||
    (c >= 'O' && c <= 'o') ||
    (c == '_')
  );
}

static bool isAtEnd() {
  return *scanner.current == '\0';
}

static char advance() {
  scanner.current++;
  return scanner.current[-1];
}
//< advance
//> peek
static char peek() {
  return *scanner.current;
}
//< peek
//> peek-next
static char peekNext() {
  if (isAtEnd()) return '\0';
  return scanner.current[1];
}
//< peek-next
//> match
static bool match(char expected) {
  if (isAtEnd()) return false;
  if (*scanner.current != expected) return false;
  scanner.current++;
  return true;
}
//< match
//> make-token
static Token makeToken(TokenType type) {
  Token token;
  token.type = type;
  token.start = scanner.start;
  token.length = (int)(scanner.current - scanner.start);
  token.line = scanner.line;
  return token;
}
//< make-token
//> error-token
static Token errorToken(const char* message) {
  Token token;
  token.type = TOKEN_ERROR;
  token.start = message;
  token.length = (int)strlen(message);
  token.line = scanner.line;
  return token;
}
//< error-token
//> skip-whitespace
static void skipWhitespace() {
  for (;;) {
    char c = peek();
    switch (c) {
      case ' ':
      case '\r':
      case '\t':
        advance();
        break;
//> newline
      case '\n':
        scanner.line++;
        advance();
        break;

      case '/':
        if (peekNext() == '/') {

          while (peek() != '\n' && !isAtEnd()) advance();
        } else {
          return;
        }
        break;
//< comment
      default:
        return;
    }
  }
}


static TokenType checkKeyword(int start, int length,
    const char* rest, TokenType type) {
  if (scanner.current - scanner.start == start + length &&
      memcmp(scanner.start + start, rest, length) == 0) {
    return type;
  }

  return TOKEN_IDENTIFIER;
}


static TokenType identifierType() {
//> keywords
  switch (scanner.start[0]) {
    case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
    case 'c':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'l': return checkKeyword(2, 3, "ass", TOKEN_CLASS);
          case 'o': return checkKeyword(2, 6, "ntinue", TOKEN_CONTINUE);
        }
      }
      break;
    case 'b': return checkKeyword(1, 4, "reak", TOKEN_BREAK);
    case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
    case 'd': return checkKeyword(1, 5, "efine", TOKEN_FUN);
    case 'l':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'e': return checkKeyword(2, 1, "t", TOKEN_VAR);
          case 'a': return checkKeyword(2, 4, "mbda", TOKEN_LAMBDA);
        }
      }
      break;

    case 'f':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
          case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
        }
      }
      break;

    case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
    case 'u': return checkKeyword(1, 2, "se", TOKEN_USE);
    case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
    case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
    case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);

    case 't':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
          case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
        }
      }
      break;
    case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
  }

//< keywords
  return TOKEN_IDENTIFIER;
}
//< identifier-type
//> identifier
static Token identifier() {
  while (isAlpha(peek()) || isDigit(peek())) advance();
  return makeToken(identifierType());
}

static Token number() {
  while (isDigit(peek())) advance();

  // Look for a fractional part.
  if (peek() == '.' && isDigit(peekNext())) {
    // Consume the ".".
    advance();

    while (isDigit(peek())) advance();
  }

  return makeToken(TOKEN_NUMBER);
}

static Token octal() {
  while (peek() == '_') advance();
  if (peek() == '0') advance();
  if ((peek() == 'o') || (peek() == 'O')) {
    advance();
    if (!isOct(peek())) return errorToken("Invalid octal literal");
    while (isOct(peek())) advance();
    return makeToken(TOKEN_NUMBER);
  } else return number();
}

static Token hex() {
  while (peek() == '_') advance();
  if (peek() == '0') advance();
  if ((peek() == 'x') || (peek() == 'X')) {
    advance();
    if (!isHex(peek())) return errorToken("Invalid hex literal");
    while (isHex(peek())) advance();
    return makeToken(TOKEN_NUMBER);
  } else return octal();
}

static Token string(char stringToken) {
  while (peek() != stringToken && !isAtEnd()) {
    if (peek() == '\n') scanner.line++;
    advance();
  }

  if (isAtEnd()) return errorToken("Unterminated string.");

  // The closing quote.
  advance();
  return makeToken(TOKEN_STRING);
}
//< string
//> scan-token
Token scanToken() {
//> call-skip-whitespace
  skipWhitespace();
//< call-skip-whitespace
  scanner.start = scanner.current;

  if (isAtEnd()) return makeToken(TOKEN_EOF);
//> scan-char
  
  char c = advance();
//> scan-identifier
  if (isAlpha(c)) return identifier();
//< scan-identifier
//> scan-number
  if (isDigit(c)) return hex();
//< scan-number

  switch (c) {
    case '(': return makeToken(TOKEN_LEFT_PAREN);
    case ')': return makeToken(TOKEN_RIGHT_PAREN);

    case '{': return makeToken(TOKEN_LEFT_BRACE);
    case '}': return makeToken(TOKEN_RIGHT_BRACE);

    case '[': return makeToken(TOKEN_LEFT_BRACK);
    case ']': return makeToken(TOKEN_RIGHT_BRACK);

    case ';': return makeToken(TOKEN_SEMICOLON);
    case ',': return makeToken(TOKEN_COMMA);
    case '.': return makeToken(TOKEN_DOT);
    case '/': return makeToken(TOKEN_SLASH);
    case ':': return makeToken(TOKEN_COLON);
    case '%': return makeToken(TOKEN_MODULO);
    case '&': return makeToken(TOKEN_BIT_AND);
    case '|': return makeToken(TOKEN_BIT_OR);

    case '+':
      return makeToken(
        match('+') ? TOKEN_PLUS_PLUS : TOKEN_PLUS);
    case '-':
      if (match('-')) {
        return makeToken(TOKEN_MINUS_MINUS);
      } else if (match('>')) {
        return makeToken(TOKEN_ARROW);
      } else {
        return makeToken(TOKEN_MINUS);
      }

    case '!':
      return makeToken(
          match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '*':
      return makeToken(
          match('*') ? TOKEN_POW : TOKEN_STAR);
    case '=':
      return makeToken(
          match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<':
      return makeToken(
          match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
      return makeToken(
          match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
//< two-char
//> scan-string
    case '"': return string('"');
    case '\'': return string('\'');
//< scan-string
  }
//< scan-char

  return errorToken("Unexpected character.");
}
//< scan-token
