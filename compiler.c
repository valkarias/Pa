#include <string.h>

#include "common.h"
#include "compiler.h"
#include "object.h"
#include "memory.h"
#include "scanner.h"

#include "libraries/library.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

//>
int innermostLoopStart = -1;
int innermostLoopScopeDepth = 0;

int listCount = 0;
int isList = false;
//<

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
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
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

typedef struct ClassCompiler {
  struct ClassCompiler* enclosing;
  bool hasSuperclass;
} ClassCompiler;


Parser parser;
Compiler* current = NULL;

ClassCompiler* currentClass = NULL;

static Chunk* currentChunk() {
  return &current->function->chunk;
}

static void errorAt(Token* token, const char* message) {
//> check-panic-mode
  if (parser.panicMode) return;
//< check-panic-mode
//> set-panic-mode
  parser.panicMode = true;
//< set-panic-mode
  fprintf(stderr, "[line %d] in '%s'", token->line, parser.library->name->chars);

  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
    // Nothing.
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
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

static void initCompiler(Compiler* compiler, FunctionType type) {

  compiler->enclosing = current;
  compiler->function = NULL;

  compiler->localCount = 0;
  compiler->lastCall = false;
  compiler->scopeDepth = 0;
  compiler->type = type;
  compiler->function = newFunction(parser.library, FUNCTION_NOT_PROTECTED, type);


  current = compiler;
  if (type != TYPE_SCRIPT) {
    current->function->name = copyString(parser.previous.start,parser.previous.length);
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
//> own-initializer-error
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
    error("Too many closure variables in function.");
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
//< mark-local-captured
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
    error("Too many local variables in function.");
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
      error("Already variable with this name in this scope.");
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


  emitBytes(OP_DEFINE_GLOBAL, global);
}

static uint8_t argumentList() {
  uint8_t argCount = 0;
  if (!check(TOKEN_RIGHT_PAREN)) {
    do {
      expression();

      if (argCount == 30) {
        error("Can't have more than 30 arguments.");
      }

      argCount++;
    } while (match(TOKEN_COMMA));
  }
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
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
  consume(TOKEN_IDENTIFIER, "Expect property name after '.'.");
  uint8_t name = identifierConstant(&parser.previous);

  if (canAssign && match(TOKEN_EQUAL)) {
    expression();
    emitBytes(OP_SET_PROPERTY, name);
//> Methods and Initializers parse-call
  } else if (match(TOKEN_LEFT_PAREN)) {
    uint8_t argCount = argumentList();
    emitBytes(OP_INVOKE, name);
    emitByte(argCount);
//< Methods and Initializers parse-call
  } else {
    emitBytes(OP_GET_PROPERTY, name);
  }

  current->lastCall = false;
  isList = false;
}

static bool isHex() {
  Token c = parser.previous;
  int isDigit = c.start[0] >= '0' && c.start[0] <= '9';

  if (isDigit) {
    if (c.start[1] == 'x') {
      return true;
    }
  }

  return false;
}


static void literal(bool canAssign) {
  switch (parser.previous.type) {
    case TOKEN_FALSE: emitByte(OP_FALSE); break;
    // case TOKEN_NIL: emitByte(OP_NIL); break;
    case TOKEN_TRUE: emitByte(OP_TRUE); break;
    default: return;
  }

  current->lastCall = false;
  isList = false;
}

static void grouping(bool canAssign) {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");

  current->lastCall = false;
  isList = false;
}

static void number(bool canAssign) {
  double value = 0;

  if (isHex()) {
    value = (double)strtol(parser.previous.start, NULL, 16);
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

static void string(bool canAssign) {
  listCount = 0;
  emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));

  listCount = parser.previous.length - 2;
  isList = true;
}

static void namedVariable(Token name, bool canAssign) {
  uint8_t getOp, setOp;
  int arg = resolveLocal(current, &name);
  if (arg != -1) {
    getOp = OP_GET_LOCAL;
    setOp = OP_SET_LOCAL;
//> Closures named-variable-upvalue
  } else if ((arg = resolveUpvalue(current, &name)) != -1) {
    getOp = OP_GET_UPVALUE;
    setOp = OP_SET_UPVALUE;
//< Closures named-variable-upvalue
  } else {
    arg = identifierConstant(&name);
    getOp = OP_GET_GLOBAL;
    setOp = OP_SET_GLOBAL;
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    expression();

    emitBytes(setOp, (uint8_t)arg);
  } else {

    emitBytes(getOp, (uint8_t)arg);
  }

  isList = skip;
}

static void variable(bool canAssign) {
  namedVariable(parser.previous, canAssign);

  current->lastCall = false;
  isList = skip;

  printf("%d\n", isList);
}

static Token syntheticToken(const char* text) {
  Token token;
  token.start = text;
  token.length = (int)strlen(text);
  return token;
}

static void super_(bool canAssign) {

  if (currentClass == NULL) {
    error("Can't use 'super' outside of a class.");
  } else if (!currentClass->hasSuperclass) {
    error("Can't use 'super' in a class with no superclass.");
  }


  consume(TOKEN_DOT, "Expect '.' after 'super'.");
  consume(TOKEN_IDENTIFIER, "Expect superclass method name.");
  uint8_t name = identifierConstant(&parser.previous);
  namedVariable(syntheticToken("this"), false);

  if (match(TOKEN_LEFT_PAREN)) {
    uint8_t argCount = argumentList();
    namedVariable(syntheticToken("super"), false);
    emitBytes(OP_SUPER_INVOKE, name);
    emitByte(argCount);
  } else {
    namedVariable(syntheticToken("super"), false);
    emitBytes(OP_GET_SUPER, name);
  }
//< super-invoke
}
static void this_(bool canAssign) {
//> this-outside-class
  if (currentClass == NULL) {
    error("Can't use 'this' outside of a class.");
    return;
  }
  
//< this-outside-class
  variable(false);
}
static void unary(bool canAssign) {
//< Global Variables unary
  TokenType operatorType = parser.previous.type;

  parsePrecedence(PREC_UNARY);

  switch (operatorType) {
    case TOKEN_BANG: emitByte(OP_NOT); break;
//< Types of Values compile-not
    case TOKEN_MINUS: emitByte(OP_NEGATE); break;
    default: return; // Unreachable.
  }

  current->lastCall = false;
  isList = false;
}

static void list(bool canAssign) {
  listCount = 0;
  if (!check(TOKEN_RIGHT_BRACK)) {
    do {
      if (check(TOKEN_RIGHT_BRACK)) {
        break;
      }

      parsePrecedence(PREC_OR);

      if (listCount == UINT8_COUNT) {
        error("Cannot have more than 256 items in a list.");
      }
      listCount++;
    } while(match(TOKEN_COMMA));
  }

  consume(TOKEN_RIGHT_BRACK, "Expect ']' after list.");

  emitByte(OP_BUILD_LIST);
  emitByte(listCount);

  current->lastCall = false;
  isList = true;
  return;
}

static void arrowFunctionArguments() {
  if (!check(TOKEN_GREATER)) {
    do {
      current->function->arity++;
      if (current->function->arity > 30) {
        errorAtCurrent("Can't have more than 30 parameters.");
      }
      uint8_t constant = parseVariable("Expect parameter name or ('>' / '{').");
      defineVariable(constant);
    } while (match(TOKEN_COMMA));
  }
}

static void subscript(bool canAssign) {
  int index = strtod(parser.previous.start, NULL);

  parsePrecedence(PREC_OR);

  if (isList == false) {
    error("Type not subscriptable.");
  }

  if (current->function->type == TYPE_SCRIPT) {
    if (isList != skip) {
      if (index < 0 || index > listCount - 1) {
        error("List index out bounds.");
      }

      consume(TOKEN_RIGHT_BRACK, "Expect ']' after index.");

      if (canAssign && match(TOKEN_EQUAL)) {
        expression();
        emitByte(OP_STORE_SUBSCR_C);
      } else {
        emitByte(OP_INDEX_SUBSCR_C);
      }

      current->lastCall = false;
      isList = false;
      return;
    }
  }
  consume(TOKEN_RIGHT_BRACK, "Expect ']' after index.");

  if (canAssign && match(TOKEN_EQUAL)) {
    expression();
    emitByte(OP_STORE_SUBSCR);
  } else {
    emitByte(OP_INDEX_SUBSCR);
  }

  current->lastCall = false;
  isList = false;
  return;
}

static void arrow() {
  arrowFunctionArguments();

  consume(TOKEN_GREATER, "Expect '>' after arguments.");

  if (match(TOKEN_LEFT_BRACE)) {
    block();
  } else {
    expression();
    emitByte(OP_RETURN);
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
  }

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
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     dot,    PREC_CALL},
  [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
  [TOKEN_BANG]          = {unary,    NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     binary, PREC_EQUALITY},

  [TOKEN_LEFT_BRACK]    = {list, subscript, PREC_SUBSCRIPT},
  [TOKEN_RIGHT_BRACK]   = {NULL, NULL, PREC_NONE},

  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COLON]         = {NULL,     NULL, PREC_NONE},

  [TOKEN_EQUAL_EQUAL]   = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_GREATER]       = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_GREATER_EQUAL] = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_LESS]          = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_LESS_EQUAL]    = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_IDENTIFIER]    = {variable, NULL,   PREC_NONE},
  [TOKEN_STRING]        = {string,   NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     and_,   PREC_AND},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_USE]           = {NULL,     NULL,   PREC_NONE},

  [TOKEN_CONTINUE]      = {NULL, NULL, PREC_NONE},
  [TOKEN_BREAK]         = {NULL, NULL, PREC_NONE},

  // [TOKEN_NIL]           = {literal,  NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     or_,    PREC_OR},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {super_,   NULL,   PREC_NONE},
  [TOKEN_THIS]          = {this_,    NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {literal,  NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

static void parsePrecedence(Precedence precedence) {

  advance();
  ParseFn prefixRule = getRule(parser.previous.type)->prefix;
  if (prefixRule == NULL) {
    error("Expect expression.");
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

  consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void function(FunctionType type) {
  Compiler compiler;
  initCompiler(&compiler, type);
  beginScope();

  consume(TOKEN_GREATER, "Expect '>' after function name.");
  arrow();
}

static void method() {
  consume(TOKEN_IDENTIFIER, "Expect method name.");
  uint8_t constant = identifierConstant(&parser.previous);

  FunctionType type = TYPE_METHOD;

  if (parser.previous.length == 4 &&
      memcmp(parser.previous.start, "init", 4) == 0) {
    type = TYPE_INITIALIZER;
  }
  
  function(type);
  emitBytes(OP_METHOD, constant);
}

static void classDeclaration() {
  consume(TOKEN_IDENTIFIER, "Expect class name.");

  Token className = parser.previous;

  uint8_t nameConstant = identifierConstant(&parser.previous);
  declareVariable();

  emitBytes(OP_CLASS, nameConstant);
  defineVariable(nameConstant);


  ClassCompiler classCompiler;

  classCompiler.hasSuperclass = false;

  classCompiler.enclosing = currentClass;
  currentClass = &classCompiler;

  if (match(TOKEN_LESS)) {
    consume(TOKEN_IDENTIFIER, "Expect superclass name.");
    variable(false);
//> inherit-self

    if (identifiersEqual(&className, &parser.previous)) {
      error("A class can't inherit from itself.");
    }

    beginScope();
    addLocal(syntheticToken("super"));
    defineVariable(0);
    
//< superclass-variable
    namedVariable(className, false);
    emitByte(OP_INHERIT);
//> set-has-superclass
    classCompiler.hasSuperclass = true;
//< set-has-superclass
  }
  

  namedVariable(className, false);
//< Methods and Initializers load-class
  consume(TOKEN_LEFT_BRACE, "Expect '{' before class body.");
//> Methods and Initializers class-body
  while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
    method();
  }

  consume(TOKEN_RIGHT_BRACE, "Expect '}' after class body.");

  emitByte(OP_POP);

  if (classCompiler.hasSuperclass) {
    endScope();
  }

  currentClass = currentClass->enclosing;

}

static void funDeclaration() {
  uint8_t global = parseVariable("Expect function name.");
  markInitialized();
  function(TYPE_FUNCTION);
  defineVariable(global);
}
static void varDeclaration() {
  uint8_t global = parseVariable("Expect variable name.");

  consume(TOKEN_EQUAL, "Expect '=' after variable name.");
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

  defineVariable(global);
}

static void expressionStatement() {
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
  emitByte(OP_POP);
}

static void breakLoop() {
  int i = innermostLoopStart;
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
  if (innermostLoopStart == -1) {
    error("Can't use 'break' outside of a loop.");
  }

  consume(TOKEN_SEMICOLON, "Expect ';' after 'break'.");
  for (int i = current->localCount - 1; i >= 0 && current->locals[i].depth > innermostLoopScopeDepth; i--) {
    emitByte(OP_POP);
  }

  emitJump(OP_BREAK);
}

static void continueStatement() {
  if (innermostLoopStart == -1) {
    error("Can't use 'continue' outside of a loop.");
  }

  consume(TOKEN_SEMICOLON, "Expect ';' after 'continue'.");
  for (int i = current->localCount - 1; i >= 0 && current->locals[i].depth > innermostLoopScopeDepth; i--) {
    emitByte(OP_POP);
  }

  emitLoop(innermostLoopStart);
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

  int MotherLoopStart = innermostLoopStart;
  int MotherLoopScopeDepth = innermostLoopScopeDepth; 

  innermostLoopStart = currentChunk()->count;
  innermostLoopScopeDepth = current->scopeDepth;
  
  int exitJump = -1;
  if (!match(TOKEN_SEMICOLON)) {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

    // Jump out of the loop if the condition is false.
    exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP); // Condition.
  }

  if (!match(TOKEN_SEMICOLON)) {
    int bodyJump = emitJump(OP_JUMP);
    int incrementStart = currentChunk()->count;
    expression();
    emitByte(OP_POP);

    emitLoop(innermostLoopStart);
    innermostLoopStart = incrementStart;
    patchJump(bodyJump);
  }
//< for-increment

  statement();
  emitLoop(innermostLoopStart);
//> exit-jump

  if (exitJump != -1) {
    patchJump(exitJump);
    emitByte(OP_POP); // Condition.
  }

  innermostLoopStart = MotherLoopStart; 
  innermostLoopScopeDepth = MotherLoopScopeDepth; 

  breakLoop();
  endScope();
//< for-end-scope
}

static void useStatement() {
  if (match(TOKEN_STRING)) {
    int constant = addConstant(currentChunk(), OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));

    emitBytes(OP_USE, constant);
    emitByte(OP_POP);
  } else {
    consume(TOKEN_IDENTIFIER, "Expect library identifier.");
    uint8_t libName = identifierConstant(&parser.previous);
    declareVariable(&parser.previous);

    int index = getNativeModule( (char*)parser.previous.start, parser.previous.length - parser.current.length );

    if (index == -1) {
      error("Native library does not exist."); 
    }

    emitBytes(OP_USE_BUILTIN, index);
    emitByte(libName);

    defineVariable(libName);
  }

  consume(TOKEN_SEMICOLON, "Expect ';' after use statement.");
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

  consume(TOKEN_SEMICOLON, "Expect ';' after value.");
}

static void returnStatement() {
  if (current->function->type == TYPE_SCRIPT) {
    error("Can not return from top-level code.");
  }

  if (current->function->type == TYPE_INITIALIZER) {
    error("Can't return a value from an initializer.");
  }

  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after return value.");


  if (current->lastCall) {
    current->function->chunk.code[current->function->chunk.count - 2] = OP_TAIL_CALL;
    current->lastCall = false;
  }

  emitByte(OP_RETURN);

  if (current->scopeDepth < 2) {
    current->function->protection = FUNCTION_PROTECTED;
  }
}

static void whileStatement() {
  beginScope();

  int MotherLoopStart = innermostLoopStart;
  innermostLoopStart = currentChunk()->count;
//< loop-start
  expression();
  int exitJump = emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP);

  statement();
//> loop
  emitLoop(innermostLoopStart);
//< loop

  patchJump(exitJump);
  emitByte(OP_POP);

  breakLoop();
  innermostLoopStart = MotherLoopStart;

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
    case OP_POP:
    case OP_EQUAL:
    case OP_GREATER:
    case OP_LESS:
    case OP_ADD:
    case OP_SUBTRACT:
    case OP_MULTIPLY:
    case OP_DIVIDE:
    case OP_NOT:
    case OP_NEGATE:
    case OP_CLOSE_UPVALUE:
    case OP_RETURN:
    case OP_BREAK:
    case OP_RECENT_USE:
      return 0;

    case OP_CONSTANT:
    case OP_GET_LOCAL:
    case OP_SET_LOCAL:
    case OP_GET_GLOBAL:
    case OP_GET_UPVALUE:
    case OP_SET_UPVALUE:
    case OP_GET_PROPERTY:
    case OP_SET_PROPERTY:
    case OP_GET_SUPER:
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
  isList = false;

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