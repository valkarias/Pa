//> Scanning on Demand scanner-h
#ifndef pcrap_scanner_h
#define pcrap_scanner_h
//> token-type

typedef enum {
  // Single-character tokens.
  TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
  TOKEN_LEFT_BRACK, TOKEN_RIGHT_BRACK,
  TOKEN_MODULO, TOKEN_POW, TOKEN_BIT_AND, TOKEN_BIT_OR, TOKEN_BIT_XOR,
  
  TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
  TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR, TOKEN_COLON,
  // One or two character tokens.
  TOKEN_BANG, TOKEN_BANG_EQUAL,
  TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER, TOKEN_GREATER_EQUAL,
  TOKEN_LESS, TOKEN_LESS_EQUAL, TOKEN_PLUS_PLUS,TOKEN_MINUS_MINUS,
  TOKEN_ARROW, TOKEN_BIT_LEFT, TOKEN_BIT_RIGHT,

  // Literals.
  TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,
  // Keywords.
  TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
  TOKEN_FOR, TOKEN_FUN, TOKEN_IF, TOKEN_OR, //TOKEN_NIL, 
  TOKEN_RETURN, TOKEN_THIS, TOKEN_ASSERT,
  TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE, TOKEN_CONTINUE, TOKEN_BREAK, TOKEN_USE,
  TOKEN_LAMBDA, TOKEN_NONE, TOKEN_PRIVATE,
 
  TOKEN_ERROR, TOKEN_EOF
} TokenType;
//< token-type
//> token-struct

typedef struct {
  TokenType type;
  const char* start;
  int length;
  int line;
} Token;
//< token-struct

void initScanner(const char* source);
//> scan-token-h
Token scanToken();
//< scan-token-h

#endif
