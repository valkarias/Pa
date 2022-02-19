#include <string.h>

#include "common.h"
#include "compiler.h"
#include "object.h"
#include "memory.h"
#include "scanner.h"

#include "vm.h"

#include "../libraries/library.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif


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
  PREC_BIT_OR,
  PREC_BIT_AND,     // &
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_EXPONENT,    // **
  PREC_SUBSCRIPT,   // []
  PREC_CALL,        // . ()
  PREC_PRIMARY
} Precedence;


typedef void (*ParseFn)(bool canAssign);

typedef struct {
  ParseFn prefix;
  ParseFn infix;
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
} Compiler;

//lol??
typedef struct ClassCompiler {
  struct ClassCompiler* enclosing;
} ClassCompiler;


Parser parser;
Compiler* current = NULL;
Static staticCheck;

ClassCompiler* currentClass = NULL;

static Chunk* currentChunk() {
  return &current->function->chunk;
}

static void errorAt(Token* token, const char* message) {
  if (parser.panicMode) return;
  parser.panicMode = true;
  fprintf(stderr, "\n%s::", parser.library->name->chars);

  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at the end");
  } else if (token->type == TOKEN_ERROR) {
    // Nothing.
  } else {
    fprintf(stderr, "%d | %.*s", token->line, token->length,token->start);
  }

  fprintf(stderr, "\n    -> %s\n", message);
  parser.hadError = true;
}

static void error(const char* message) {
  errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char* message) {
  errorAt(&parser.current, message);
}

static void advance() {
  parser.previous = parser.current;

  for (;;) {
    parser.current = scanToken();
    if (parser.current.type != TOKEN_ERROR) break;

    errorAtCurrent(parser.current.start);
  }
}

static void consume(TokenType type, const char* message) {
  if (parser.current.type == type) {
    advance();
    return;
  }

  errorAtCurrent(message);
}

static bool check(TokenType type) {
  return parser.current.type == type;
}

static bool match(TokenType type) {
  if (!check(type)) return false;
  advance();
  return true;
}

static void emitByte(uint8_t byte) {
  writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
  emitByte(byte1);
  emitByte(byte2);
}

static void emitLoop(int loopStart) {
  emitByte(OP_LOOP);

  int offset = currentChunk()->count - loopStart + 2;
  if (offset > UINT16_MAX) error("Loop body too large.");

  emitByte((offset >> 8) & 0xff);
  emitByte(offset & 0xff);
}

static int emitJump(uint8_t instruction) {
  emitByte(instruction);
  emitByte(0xff);
  emitByte(0xff);
  return currentChunk()->count - 2;
}

static void emitReturn() {
  if (current->function->type == TYPE_INITIALIZER) {
    emitBytes(OP_GET_LOCAL, 0);
  } else {
    emitByte(OP_NIL);
  }

  emitByte(OP_RETURN);
}


static uint8_t makeConstant(Value value) {
  int constant = addConstant(currentChunk(), value);
  if (constant > UINT8_MAX) {
    error("Too many constants in one chunk.");
    return 0;
  }

  return (uint8_t)constant;
}

static void emitConstant(Value value) {
  emitBytes(OP_CONSTANT, makeConstant(value));
}


static void patchJump(int offset) {
  int jump = currentChunk()->count - offset - 2;

  if (jump > UINT16_MAX) {
    error("Too much code to jump over.");
  }

  currentChunk()->code[offset] = (jump >> 8) & 0xff;
  currentChunk()->code[offset + 1] = jump & 0xff;
}

static void initStaticChecks(Static* s) {
  s->innermostLoopStart = -1;
  s->innermostLoopScopeDepth = 0;
}

static void initCompiler(Compiler* compiler, FunctionType type) {
  compiler->enclosing = current;
  compiler->function = NULL;

  compiler->localCount = 0;
  compiler->lastCall = false;
  compiler->scopeDepth = 0;


  compiler->type = type;
  compiler->function = newFunction(parser.library, type);

  current = compiler;
  switch (type) {
    case TYPE_FUNCTION:
    case TYPE_METHOD:
    case TYPE_INITIALIZER:
      current->function->name = copyString(parser.previous.start,parser.previous.length);
      break;

    case TYPE_UNKNOWN:
      current->function->name = copyString("unknown", 7);
      break;

    case TYPE_SCRIPT:
      current->function->name = NULL;
      break;
  }

  Local* local = &current->locals[current->localCount++];
  local->depth = 0;

  local->isCaptured = false;

  if (type != TYPE_FUNCTION) {
    local->name.start = "this";
    local->name.length = 4;
  } else {
    local->name.start = "";
    local->name.length = 0;
  }

}

static ObjFunction* endCompiler() {
  emitReturn();
  ObjFunction* function = current->function;

#ifdef DEBUG_PRINT_CODE
  if (!parser.hadError) {
    disassembleChunk(currentChunk(), function->name != NULL ? function->name->chars : function->library->name->chars);
  }
#endif

  current = current->enclosing;
  return function;
}

static void beginScope() {
  current->scopeDepth++;
}

static void endScope() {
  current->scopeDepth--;
//> pop-locals

  while (current->localCount > 0 &&
         current->locals[current->localCount - 1].depth >
            current->scopeDepth) {

    if (current->locals[current->localCount - 1].isCaptured) {
      emitByte(OP_CLOSE_UPVALUE);
    } else {
      emitByte(OP_POP);
    }
//< Closures end-scope
    current->localCount--;
  }
//< pop-locals
}


static int getArgCount(uint8_t *code, const ValueArray constants, int ip);

static void expression();
static void block();
static void body();
static void statement();
static void declaration();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

static uint8_t identifierConstant(Token* name) {
  return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

static bool identifiersEqual(Token* a, Token* b) {
  if (a->length != b->length) return false;
  return memcmp(a->start, b->start, a->length) == 0;
}

static int resolveLocal(Compiler* compiler, Token* name) {
  for (int i = compiler->localCount - 1; i >= 0; i--) {
    Local* local = &compiler->locals[i];
    if (identifiersEqual(name, &local->name)) {
      if (local->depth == -1) {
        error("Can't read local variable in its own initializer.");
      }
//< own-initializer-error
      return i;
    }
  }

  return -1;
}

static int addUpvalue(Compiler* compiler, uint8_t index,
                      bool isLocal) {
  int upvalueCount = compiler->function->upvalueCount;
//> existing-upvalue

  for (int i = 0; i < upvalueCount; i++) {
    Upvalue* upvalue = &compiler->upvalues[i];
    if (upvalue->index == index && upvalue->isLocal == isLocal) {
      return i;
    }
  }


  if (upvalueCount == UINT8_COUNT) {
    error("Too many closure variables in one function.");
    return 0;
  }

//< too-many-upvalues
  compiler->upvalues[upvalueCount].isLocal = isLocal;
  compiler->upvalues[upvalueCount].index = index;
  return compiler->function->upvalueCount++;
}

static int resolveUpvalue(Compiler* compiler, Token* name) {
  if (compiler->enclosing == NULL) return -1;

  int local = resolveLocal(compiler->enclosing, name);
  if (local != -1) {
    compiler->enclosing->locals[local].isCaptured = true;
    return addUpvalue(compiler, (uint8_t)local, true);
  }

//> resolve-upvalue-recurse
  int upvalue = resolveUpvalue(compiler->enclosing, name);
  if (upvalue != -1) {
    return addUpvalue(compiler, (uint8_t)upvalue, false);
  }
  
//< resolve-upvalue-recurse
  return -1;
}

static void addLocal(Token name) {
//> too-many-locals
  if (current->localCount == UINT8_COUNT) {
    error("Too many local variables in one function.");
    return;
  }

//< too-many-locals
  Local* local = &current->locals[current->localCount++];
  local->name = name;

  local->depth = -1;

  local->isCaptured = false;
//< Closures init-is-captured
}


static void declareVariable() {
  if (current->scopeDepth == 0) return;

  Token* name = &parser.previous;
//> existing-in-scope
  for (int i = current->localCount - 1; i >= 0; i--) {
    Local* local = &current->locals[i];
    if (local->depth != -1 && local->depth < current->scopeDepth) {
      break; 
    }
    
    if (identifiersEqual(name, &local->name)) {
      error("Already a defined variable with this name in this scope.");
    }
  }

//< existing-in-scope
  addLocal(*name);
}

static uint8_t parseVariable(const char* errorMessage) {
  consume(TOKEN_IDENTIFIER, errorMessage);

  declareVariable();
  if (current->scopeDepth > 0) return 0;

  return identifierConstant(&parser.previous);
}

static void markInitialized() {
  if (current->scopeDepth == 0) return;

  current->locals[current->localCount - 1].depth = current->scopeDepth;
}

static void defineVariable(uint8_t global) {

  if (current->scopeDepth > 0) {
    markInitialized();
    return;
  }

  emitBytes(OP_DEFINE_LIBRARY, global);
}

static uint8_t argumentList() {
  uint8_t argCount = 0;
  if (!check(TOKEN_RIGHT_PAREN)) {
    do {
      expression();

      if (argCount == 30) {
        error("Can't have more than 30 arguments within a function.");
      }

      argCount++;
    } while (match(TOKEN_COMMA));
  }
  consume(TOKEN_RIGHT_PAREN, "Expected a ')' after arguments.");
  return argCount;
}

static void and_(bool canAssign) {
  int endJump = emitJump(OP_JUMP_IF_FALSE);

  emitByte(OP_POP);
  parsePrecedence(PREC_AND);

  patchJump(endJump);

  current->lastCall = false;
}

static void binary(bool canAssign) {
//< Global Variables binary
  TokenType operatorType = parser.previous.type;
  ParseRule* rule = getRule(operatorType);
  parsePrecedence((Precedence)(rule->precedence + 1));

  switch (operatorType) {
//> Types of Values comparison-operators
    case TOKEN_BANG_EQUAL:    emitBytes(OP_EQUAL, OP_NOT); break;
    case TOKEN_EQUAL_EQUAL:   emitByte(OP_EQUAL); break;
    case TOKEN_GREATER:       emitByte(OP_GREATER); break;
    case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
    case TOKEN_LESS:          emitByte(OP_LESS); break;
    case TOKEN_LESS_EQUAL:    emitBytes(OP_GREATER, OP_NOT); break;
//< Types of Values comparison-operators
    case TOKEN_PLUS:          emitByte(OP_ADD); break;
    case TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
    case TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
    case TOKEN_SLASH:         emitByte(OP_DIVIDE); break;
    case TOKEN_MODULO:        emitByte(OP_MOD); break;
    case TOKEN_POW:           emitByte(OP_POW); break;
    case TOKEN_BIT_AND:       emitByte(OP_BIT_AND); break;
    case TOKEN_BIT_OR:        emitByte(OP_BIT_OR); break;
    default: return; // Unreachable.
  }

  current->lastCall = false;
}

static void call(bool canAssign) {
  uint8_t argCount = argumentList();

  emitBytes(OP_CALL, argCount);
  current->lastCall = true;
}

static void dot(bool canAssign) {
  consume(TOKEN_IDENTIFIER, "Expected a property name after '.'");
  uint8_t name = identifierConstant(&parser.previous);

  if (canAssign && match(TOKEN_EQUAL)) {
    expression();
    emitBytes(OP_SET_PROPERTY, name);

  } else if (canAssign && match(TOKEN_PLUS_PLUS)) {
    emitBytes(OP_GET_PROPERTY_NO_POP, name);
    emitByte(OP_INCREMENT);
    emitBytes(OP_SET_PROPERTY, name);

  } else if (canAssign && match(TOKEN_MINUS_MINUS)) {
    emitBytes(OP_GET_PROPERTY_NO_POP, name);
    emitByte(OP_DECREMENT);
    emitBytes(OP_SET_PROPERTY, name);

  } else if (match(TOKEN_LEFT_PAREN)) {
    uint8_t argCount = argumentList();
    emitBytes(OP_INVOKE, name);
    emitByte(argCount);

  } else {
    emitBytes(OP_GET_PROPERTY, name);
  }

  current->lastCall = false;
  
}

static bool isHex() {
  Token c = parser.previous;
  int isDigit = c.start[0] >= '0' && c.start[0] <= '9';

  if (isDigit) {
    if (c.start[1] == 'x' || c.start[1] == 'X') {
      return true;
    }
  }

  return false;
}

static bool isOct() {
  Token c = parser.previous;
  int isDigit = c.start[0] >= '0' && c.start[0] <= '9';

  if (isDigit) {
    if (c.start[1] == 'o' || c.start[1] == 'O') {
      return true;
    }
  }

  return false;
}

//from the trashcan.
static double toOct(const char* c) {
  long int oc = 0;
  char* temp = (char*)c;

  for (int i = 0; i < strlen(c); i++) {
    if (c[i] == ';' || c[i] == '\n') {
      temp[i] = '\0';
    }
  }
  
  for (int i = 0; i < strlen(c); i++) {
    if (c[i] == 'o' || c[i] == 'O') {
      temp += 2;
    }
  }

  oc = strtol(temp, NULL, 8);
  return (double)oc;
}

static void literal(bool canAssign) {
  switch (parser.previous.type) {
    case TOKEN_FALSE: emitByte(OP_FALSE); break;
    case TOKEN_TRUE: emitByte(OP_TRUE); break;
    case TOKEN_NONE: emitByte(OP_NIL); break;
    default: return;
  }

  current->lastCall = false;
  
}

static void grouping(bool canAssign) {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expected a ')' after the expression.");
  current->lastCall = false;
}

static void number(bool canAssign) {
  double value = 0;

  if (isHex()) {
    value = (double)strtol(parser.previous.start, NULL, 16);
  } else if (isOct()) { 
    value = toOct(parser.previous.start);
  } else {
    value = strtod(parser.previous.start, NULL);
  }

  emitConstant(NUMBER_VAL(value));
}

static void or_(bool canAssign) {
  int elseJump = emitJump(OP_JUMP_IF_FALSE);
  int endJump = emitJump(OP_JUMP);

  patchJump(elseJump);
  emitByte(OP_POP);

  parsePrecedence(PREC_OR);
  patchJump(endJump);

  current->lastCall = false;
  
}

static int analyzeString(char* str, int len) {
  for (int i = 0; i < len - 1; i++) {
    if (str[i] == '\\') {
      switch(str[i + 1]) {
        case 'n': str[i + 1] = '\n'; break;
        case 'a': str[i + 1] = '\a'; break;
        case 't': str[i + 1] = '\t'; break;
        case 'r': str[i + 1] = '\r'; break;
        case '\\': str[i + 1] = '\\'; break;

        case '\'':
        case '"': {
          break;
        }

        default:
          continue;
      }

      memmove(&str[i], &str[i + 1], len - i);
      len -= 1;
    }

  }

  return len;
}

static Value refine(bool canAssign) {
  int len = parser.previous.length - 2;

  char* buffer = ALLOCATE(char, len + 1);
  memcpy(buffer, parser.previous.start + 1, len);
  int refinedLen = analyzeString(buffer, len);

  if (refinedLen != len) {
    buffer = GROW_ARRAY(char, buffer, len + 1, refinedLen + 1);
  }
  buffer[refinedLen] = '\0';

  return OBJ_VAL(takeString(buffer, refinedLen));
}

static void string(bool canAssign) {
  emitConstant(refine(canAssign));
}

static void namedVariable(Token name, bool canAssign) {
  uint8_t getOp, setOp;
  int arg = resolveLocal(current, &name);
  if (arg != -1) {
    getOp = OP_GET_LOCAL;
    setOp = OP_SET_LOCAL;
  } else if ((arg = resolveUpvalue(current, &name)) != -1) {
    getOp = OP_GET_UPVALUE;
    setOp = OP_SET_UPVALUE;
  } else {
    arg = identifierConstant(&name);
    ObjString* string = copyString(name.start, name.length);
    Value val;
    if (tableGet(&vm.globals, string, &val)) {
      getOp = OP_GET_GLOBAL;
      canAssign = false;
    } else {
      getOp = OP_GET_LIBRARY;
      setOp = OP_SET_LIBRARY;
    }
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    expression();
    emitBytes(setOp, (uint8_t)arg);
  } else if (canAssign && match(TOKEN_PLUS_PLUS)) {
    namedVariable(name, false);
    emitByte(OP_INCREMENT);
    emitBytes(setOp, (uint8_t)arg);

  } else if (canAssign && match(TOKEN_MINUS_MINUS)) {
    namedVariable(name, false);
    emitByte(OP_DECREMENT);
    emitBytes(setOp, (uint8_t)arg);

  } else {
    emitBytes(getOp, (uint8_t)arg);
  }

  
}

static void variable(bool canAssign) {
  namedVariable(parser.previous, canAssign);
  current->lastCall = false;
}

static Token syntheticToken(const char* text) {
  Token token;
  token.start = text;
  token.length = (int)strlen(text);
  return token;
}

static void this_(bool canAssign) {
  if (currentClass == NULL) {
    error("Can't use the keyword 'this' outside of a class.");
    return;
  }
  
  variable(false);
}
static void unary(bool canAssign) {
//< Global Variables unary
  TokenType operatorType = parser.previous.type;

  parsePrecedence(PREC_UNARY);

  switch (operatorType) {
    case TOKEN_BANG: emitByte(OP_NOT); break;
    case TOKEN_MINUS: emitByte(OP_NEGATE); break;
    default: return; // Unreachable.
  }

  current->lastCall = false;
  
}

static void increment(bool canAssign) {
  emitByte(OP_INCREMENT);
}

static void decrement(bool canAssign) {
  emitByte(OP_DECREMENT);
}

static void list(bool canAssign) {
  int count = 0;

  if (!check(TOKEN_RIGHT_BRACK)) {
    do {
      if (check(TOKEN_RIGHT_BRACK)) {
        break;
      }

      parsePrecedence(PREC_OR);

      count++;
    } while(match(TOKEN_COMMA));
  }

  consume(TOKEN_RIGHT_BRACK, "Expected a ']' after list.");
  emitBytes(OP_BUILD_LIST, count);
  current->lastCall = false;
  
  
  return;
}

static void functionArguments() {
  if (!check(TOKEN_RIGHT_PAREN)) {
    do {
      current->function->arity++;
      if (current->function->arity > 30) {
        errorAtCurrent("Can't have more than 30 parameters.");
      }
      uint8_t constant = parseVariable("Expected a parameter name or ')'.");
      defineVariable(constant);
    } while (match(TOKEN_COMMA));
  }
}

static void subscript(bool canAssign) {
  parsePrecedence(PREC_OR);

  consume(TOKEN_RIGHT_BRACK, "Expected a ']' after the index value.");

  if (canAssign && match(TOKEN_EQUAL)) {
    expression();
    emitByte(OP_STORE_SUBSCR);
  } else if (canAssign && match(TOKEN_PLUS_PLUS)) {
    emitBytes(OP_INDEX_SUBSCR_NO_POP, OP_INCREMENT);
    emitByte(OP_STORE_SUBSCR);

  } else if (canAssign && match(TOKEN_MINUS_MINUS)) {
    emitBytes(OP_INDEX_SUBSCR_NO_POP, OP_DECREMENT);
    emitByte(OP_STORE_SUBSCR);
    
  } else {
    emitByte(OP_INDEX_SUBSCR);
  }

  current->lastCall = false;
  
  return;
}

static void arrow(bool canAssign) {
  Compiler compiler;
  initCompiler(&compiler, TYPE_UNKNOWN);
  beginScope();

  consume(TOKEN_LEFT_PAREN, "Expected a '(' after the 'lambda' keyword.");
  functionArguments();
  consume(TOKEN_RIGHT_PAREN, "Expected a ')'.");

  consume(TOKEN_ARROW, "Expected an arrow ('->') after ')'.");

  if (match(TOKEN_LEFT_BRACE)) {
    block();
  } else {
    expression();
    emitByte(OP_RETURN);
  }

  ObjFunction* function = endCompiler();

  uint8_t constant = makeConstant(OBJ_VAL(function));
  emitBytes(OP_CLOSURE, constant);

  for (int i = 0; i < function->upvalueCount; i++) {
    emitByte(current->upvalues[i].isLocal ? 1 : 0);
    emitByte(current->upvalues[i].index);
  }
}

static void body() {
  functionArguments();

  consume(TOKEN_RIGHT_PAREN, "Expected a ')'.");
  consume(TOKEN_LEFT_BRACE, "Expected a '{' after the closing ')'.");

  block();

  ObjFunction* function = endCompiler();
  uint8_t constant = makeConstant(OBJ_VAL(function));
  emitBytes(OP_CLOSURE, constant);

  for (int i = 0; i < function->upvalueCount; i++) {
    emitByte(current->upvalues[i].isLocal ? 1 : 0);
    emitByte(current->upvalues[i].index);
  }
}

ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {grouping, call,   PREC_CALL},
  [TOKEN_RIGHT_PAREN]   = NONE,
  [TOKEN_LEFT_BRACE]    = NONE, 
  [TOKEN_RIGHT_BRACE]   = NONE,
  [TOKEN_COMMA]         = NONE,
  [TOKEN_DOT]           = {NULL,     dot,    PREC_CALL},
  [TOKEN_SEMICOLON]     = NONE,
  [TOKEN_POW]           = {NULL,     binary, PREC_EXPONENT},
  [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
  [TOKEN_MODULO]        = {NULL,     binary, PREC_FACTOR},
  [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
  [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
  [TOKEN_BIT_AND]       = {NULL,     binary, PREC_BIT_AND},
  [TOKEN_BIT_OR]        = {NULL,     binary, PREC_BIT_OR},
  [TOKEN_BANG]          = {unary,    NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_LEFT_BRACK]    = {list, subscript, PREC_SUBSCRIPT},
  [TOKEN_RIGHT_BRACK]   = {NULL, NULL, PREC_NONE},
  [TOKEN_EQUAL]         = NONE,
  [TOKEN_COLON]         = NONE,
  [TOKEN_EQUAL_EQUAL]   = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_GREATER]       = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_GREATER_EQUAL] = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_LESS]          = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_LESS_EQUAL]    = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_IDENTIFIER]    = {variable, NULL,   PREC_NONE},
  [TOKEN_STRING]        = {string,   NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     and_,   PREC_AND},
  [TOKEN_CLASS]         = NONE,
  [TOKEN_ELSE]          = NONE,
  [TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
  [TOKEN_FOR]           = NONE,
  [TOKEN_FUN]           = NONE,
  [TOKEN_LAMBDA]        = {arrow,    NULL,   PREC_NONE},
  [TOKEN_ARROW]         = NONE,
  [TOKEN_IF]            = NONE,
  [TOKEN_USE]           = NONE,
  [TOKEN_PLUS_PLUS]     = {NULL,     increment, PREC_TERM},
  [TOKEN_MINUS_MINUS]   = {NULL,     decrement, PREC_TERM},
  [TOKEN_CONTINUE]      = NONE,
  [TOKEN_BREAK]         = NONE,
  [TOKEN_OR]            = {NULL,     or_,    PREC_OR},
  [TOKEN_PRINT]         = NONE,
  [TOKEN_RETURN]        = NONE,
  [TOKEN_THIS]          = {this_, NULL, PREC_NONE},
  [TOKEN_TRUE]          = {literal,  NULL,   PREC_NONE},
  [TOKEN_NONE]          = {literal,  NULL,   PREC_NONE},
  [TOKEN_VAR]           = NONE,
  [TOKEN_WHILE]         = NONE,
  [TOKEN_ERROR]         = NONE,
  [TOKEN_EOF]           = NONE,
};

static void parsePrecedence(Precedence precedence) {

  advance();
  ParseFn prefixRule = getRule(parser.previous.type)->prefix;
  if (prefixRule == NULL) {
    error("Expected an expression.");
    return;
  }

  bool canAssign = precedence <= PREC_ASSIGNMENT;
  prefixRule(canAssign);

  while (precedence <= getRule(parser.current.type)->precedence) {
    advance();
    ParseFn infixRule = getRule(parser.previous.type)->infix;

    infixRule(canAssign);
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    error("Invalid assignment target.");
  }

}

static ParseRule* getRule(TokenType type) {
  return &rules[type];
}

static void expression() {
  parsePrecedence(PREC_ASSIGNMENT);
}

static void block() {
  while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
    declaration();
  }

  consume(TOKEN_RIGHT_BRACE, "Expected a closing '}' after block.");
}

static void function(FunctionType type) {
  Compiler compiler;
  initCompiler(&compiler, type);
  consume(TOKEN_LEFT_PAREN, "Expected a '(' after function name.");
  beginScope();
  body();
}

static void method() {
  consume(TOKEN_IDENTIFIER, "Expected a method name.");
  uint8_t constant = identifierConstant(&parser.previous);

  FunctionType type = TYPE_METHOD;

  if (parser.previous.length == 4 && memcmp(parser.previous.start, "init", 4) == 0) {
    type = TYPE_INITIALIZER;
  }
  
  function(type);
  emitBytes(OP_METHOD, constant);
}

static void classDeclaration() {
  consume(TOKEN_IDENTIFIER, "Expected a class name.");
  Token className = parser.previous;
  uint8_t nameConstant = identifierConstant(&parser.previous);
  declareVariable();

  emitBytes(OP_CLASS, nameConstant);
  defineVariable(nameConstant);

  ClassCompiler classCompiler;
  classCompiler.enclosing = currentClass;
  currentClass = &classCompiler;

  namedVariable(className, false);
  consume(TOKEN_LEFT_BRACE, "Expected an opening '{' before the class body.");
  while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
    method();
  }

  consume(TOKEN_RIGHT_BRACE, "Expected a closing '}' after the class body.");
  emitByte(OP_POP);

  currentClass = currentClass->enclosing;
}

static void funDeclaration() {
  uint8_t global = parseVariable("Expected a function name.");
  markInitialized();
  function(TYPE_FUNCTION);
  defineVariable(global);
}

static void varDeclaration() {
  uint8_t global = parseVariable("Expected a variable name.");

  consume(TOKEN_EQUAL, "Expected an '=' after the variable name.");
  expression();
  consume(TOKEN_SEMICOLON, "Expected a ';' after variable declaration.");

  defineVariable(global);
}

static void expressionStatement() {
  expression();
  consume(TOKEN_SEMICOLON, "Expected a ';' after the previous expression.");
  emitByte(OP_POP);
}

static void breakLoop() {
  int i = staticCheck.innermostLoopStart;
  while (i < current->function->chunk.count) {
    if (current->function->chunk.code[i] == OP_BREAK) {
      current->function->chunk.code[i] = OP_JUMP;
      patchJump(i + 1);
      i += 3;
    } else {
      i += 1 + getArgCount(current->function->chunk.code, current->function->chunk.constants, i);
    }
  }
}
static void breakStatement() {
  if (staticCheck.innermostLoopStart == -1) {
    error("Can't use the keyword 'break' outside of a loop.");
  }

  consume(TOKEN_SEMICOLON, "Expected a ';' after the keyword 'break'.");
  for (int i = current->localCount - 1; i >= 0 && current->locals[i].depth > staticCheck.innermostLoopScopeDepth; i--) {
    emitByte(OP_POP);
  }

  emitJump(OP_BREAK);
}

static void continueStatement() {
  if (staticCheck.innermostLoopStart == -1) {
    error("Can't use the keyword 'continue' outside of a loop.");
  }

  consume(TOKEN_SEMICOLON, "Expected a ';' after keyword 'continue'.");
  for (int i = current->localCount - 1; i >= 0 && current->locals[i].depth > staticCheck.innermostLoopScopeDepth; i--) {
    emitByte(OP_POP);
  }

  emitLoop(staticCheck.innermostLoopStart);
}

static void forStatement() {
//> for-begin-scope
  beginScope();
//> for-initializer
  if (match(TOKEN_SEMICOLON)) {
    // No initializer.
  } else if (match(TOKEN_VAR)) {
    varDeclaration();
  } else {
    expressionStatement();
  }
//< for-initializer

  int MotherLoopStart = staticCheck.innermostLoopStart;
  int MotherLoopScopeDepth = staticCheck.innermostLoopScopeDepth; 

  staticCheck.innermostLoopStart = currentChunk()->count;
  staticCheck.innermostLoopScopeDepth = current->scopeDepth;
  
  int exitJump = -1;
  if (!match(TOKEN_SEMICOLON)) {
    expression();
    consume(TOKEN_SEMICOLON, "Expected a ';' after the loop condition.");

    // Jump out of the loop if the condition is false.
    exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP); // Condition.
  }

  if (!match(TOKEN_SEMICOLON)) {
    int bodyJump = emitJump(OP_JUMP);
    int incrementStart = currentChunk()->count;
    expression();
    emitByte(OP_POP);

    emitLoop(staticCheck.innermostLoopStart);
    staticCheck.innermostLoopStart = incrementStart;
    patchJump(bodyJump);
  }
//< for-increment

  statement();
  emitLoop(staticCheck.innermostLoopStart);
//> exit-jump

  if (exitJump != -1) {
    patchJump(exitJump);
    emitByte(OP_POP); // Condition.
  }

  breakLoop();
  endScope();

  staticCheck.innermostLoopScopeDepth = MotherLoopScopeDepth; 
  staticCheck.innermostLoopStart = MotherLoopStart; 
}

static void useStatement() {
  if (match(TOKEN_STRING)) {
    int constant = addConstant(currentChunk(), OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));


    emitBytes(OP_USE, constant);
    emitByte(OP_POP);
    
    if (match(TOKEN_FOR)) {
      uint8_t library = parseVariable("Expected an indentifier after library's path.");
      emitByte(OP_USE_NAME);
      defineVariable(library);
    }
  } else {
    consume(TOKEN_IDENTIFIER, "Expected a library's name identifier.");
    uint8_t libName = identifierConstant(&parser.previous);
    declareVariable();

    int index = getNativeModule( (char*)parser.previous.start, parser.previous.length - parser.current.length );

    if (index == -1) {
      error("Native library does not exist."); 
    }

    emitBytes(OP_USE_BUILTIN, index);
    emitByte(libName);

    defineVariable(libName);
  }

  consume(TOKEN_SEMICOLON, "Expected a ';' after the 'use' statement.");
  emitByte(OP_RECENT_USE);
}

static void ifStatement() {
  expression();

  int thenJump = emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP);

  statement();

  int elseJump = emitJump(OP_JUMP);

  patchJump(thenJump);
  emitByte(OP_POP);

  if (match(TOKEN_ELSE)) statement();
  patchJump(elseJump);
}

static void printStatement() {
  do {
    expression();
    emitByte(OP_PRINT);
  } while(match(TOKEN_COMMA));

  consume(TOKEN_SEMICOLON, "Expected a ';' after 'print' statement's value.");
}

static void returnStatement() {
  if (current->function->type == TYPE_SCRIPT) {
    error("Can not return from top-level code.");
  }

  if (current->function->type == TYPE_INITIALIZER) {
    error("Can't return a value from an initializer.");
  }

  expression();
  consume(TOKEN_SEMICOLON, "Expected a ';' after the return value.");


  if (current->lastCall) {
    current->function->chunk.code[current->function->chunk.count - 2] = OP_TAIL_CALL;
    current->lastCall = false;
  }

  emitByte(OP_RETURN);
}

static void whileStatement() {
  beginScope();

  int MotherLoopStart = staticCheck.innermostLoopStart;
  staticCheck.innermostLoopStart = currentChunk()->count;
//< loop-start
  expression();
  int exitJump = emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP);

  statement();
//> loop
  emitLoop(staticCheck.innermostLoopStart);
//< loop

  patchJump(exitJump);
  emitByte(OP_POP);

  breakLoop();
  staticCheck.innermostLoopStart = MotherLoopStart;

  endScope();
}
static void synchronize() {
  parser.panicMode = false;

  while (parser.current.type != TOKEN_EOF) {
    if (parser.previous.type == TOKEN_SEMICOLON) return;
    switch (parser.current.type) {
      case TOKEN_CLASS:
      case TOKEN_FUN:
      case TOKEN_VAR:
      case TOKEN_FOR:
      case TOKEN_BREAK:
      case TOKEN_IF:
      case TOKEN_WHILE:
      case TOKEN_PRINT:
      case TOKEN_USE:
      case TOKEN_RETURN:
        return;

      default:
        ; // Do nothing.
    }

    advance();
  }
}


static void declaration() {
  if (match(TOKEN_CLASS)) {
    classDeclaration();

  } else if (match(TOKEN_FUN)) {
    funDeclaration();
  } else if (match(TOKEN_VAR)) {
    varDeclaration();
  } else {
    statement();
  }

  if (parser.panicMode) synchronize();
}


static int getArgCount(uint8_t *code, const ValueArray constants, int ip) {
  switch (code[ip]) {
    case OP_NIL:
    case OP_TRUE:
    case OP_FALSE:
    case OP_BUILD_LIST:

    case OP_INDEX_SUBSCR:
    case OP_STORE_SUBSCR:
    case OP_INDEX_SUBSCR_NO_POP:

    case OP_POP:
    case OP_EQUAL:
    case OP_GREATER:
    case OP_LESS:
    case OP_ADD:
    case OP_SUBTRACT:
    case OP_MULTIPLY:
    case OP_DIVIDE:
    case OP_POW:
    case OP_MOD:
    case OP_NOT:
    case OP_NEGATE:
    case OP_CLOSE_UPVALUE:
    case OP_RETURN:
    case OP_BREAK:
    case OP_RECENT_USE:
    case OP_USE_NAME:
    case OP_INCREMENT:
    case OP_DECREMENT:
      return 0;

    case OP_CONSTANT:
    case OP_GET_LOCAL:
    case OP_SET_LOCAL:
    case OP_GET_GLOBAL:
    case OP_GET_LIBRARY:
    case OP_SET_LIBRARY:
    case OP_DEFINE_LIBRARY:
    case OP_GET_UPVALUE:
    case OP_SET_UPVALUE:
    case OP_GET_PROPERTY:
    case OP_SET_PROPERTY:
    case OP_GET_PROPERTY_NO_POP:
    case OP_CALL:
    case OP_TAIL_CALL:
    case OP_USE:
    case OP_METHOD:
      return 1;

    case OP_JUMP:
    case OP_JUMP_IF_FALSE:
    case OP_LOOP:
    case OP_INVOKE:
    case OP_CLASS:
    case OP_USE_BUILTIN:
      return 2;


    case OP_CLOSURE: {
      int constant = code[ip + 1];
      ObjFunction* loadedFn = AS_FUNCTION(constants.values[constant]);

      return 1 + (loadedFn->upvalueCount * 2);
    }
  }

  return 0;
}


static void statement() {
  current->lastCall = false;
  

  if (match(TOKEN_PRINT)) {
    printStatement();

  } else if (match(TOKEN_FOR)) {
    forStatement();

  } else if (match(TOKEN_USE)) {
    useStatement();
  } else if (match(TOKEN_IF)) {
    beginScope();
    ifStatement();
    endScope();
  } else if (match(TOKEN_BREAK)) {
    breakStatement();
  } else if (match(TOKEN_CONTINUE)) {
    continueStatement();
  } else if (match(TOKEN_RETURN)) {
    returnStatement();
  } else if (match(TOKEN_WHILE)) {
    whileStatement();

  } else if (match(TOKEN_LEFT_BRACE)) {
    beginScope();
    block();
    endScope();

  } else {
    expressionStatement();
  }
}

ObjFunction* compile(const char* source, ObjLibrary* library) {
  initStaticChecks(&staticCheck);

  parser.library = library;
  parser.hadError = false;
  parser.panicMode = false;

  initScanner(source);
  Compiler compiler;

  initCompiler(&compiler, TYPE_SCRIPT);
  advance();

  while (!match(TOKEN_EOF)) {
    declaration();
  }

  ObjFunction* function = endCompiler();
  return parser.hadError ? NULL : function;
}

void markCompilerRoots() {
  Compiler* compiler = current;
  while (compiler != NULL) {
    markObject((Obj*)compiler->function);
    compiler = compiler->enclosing;
  }
}