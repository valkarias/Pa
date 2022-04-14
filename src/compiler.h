//> Scanning on Demand compiler-h
#ifndef Pa_compiler_h
#define Pa_compiler_h

#include "object.h"
#include <ctype.h>

#include "scanner.h"

typedef struct {
  int innermostLoopStart;
  int innermostLoopScopeDepth;
} Static;

typedef struct {
  Token current;
  Token previous;

  bool hadError;

  bool panicMode;

  ObjLibrary* library;
} Parser;
//> precedence

typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_BIT_OR,      // |
  PREC_BIT_XOR,     // ^
  PREC_BIT_AND,     // &
  PREC_BIT_SHIFT, //<< >>
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_EXPONENT,    // **
  PREC_SUBSCRIPT,   // []
  PREC_CALL,        // . ()
  PREC_PRIMARY
} Precedence;

typedef void (*ParsePrefix)(bool canAssign);
typedef void (*ParseInfix)(bool canAssign, Token previous);

typedef struct {
  ParsePrefix prefix;
  ParseInfix infix;
  Precedence precedence;
} ParseRule;

typedef struct {
  Token name;
  int depth;

  bool isCaptured;
} Local;

typedef struct {
  uint8_t index;
  bool isLocal;
} Upvalue;

typedef struct Compiler {
  struct Compiler* enclosing;

  ObjFunction* function;
  FunctionType type;

  bool lastCall;

  Local locals[UINT8_COUNT];
  int localCount;

  Upvalue upvalues[UINT8_COUNT];

  int scopeDepth;

  Table cacheConstants;
} Compiler;

typedef struct ClassCompiler {
  Table privateVariables;
  struct ClassCompiler* enclosing;
} ClassCompiler;

#define MAX_NUMBER_VALUE 1000000
#define skip -1
#define NONE {NULL,     NULL,   PREC_NONE}


ObjFunction* compile(const char* source, ObjLibrary* library);
void markCompilerRoots();

#endif
