//> Chunks of Bytecode chunk-h
#ifndef pcrap_chunk_h
#define pcrap_chunk_h

#include "common.h"
//> chunk-h-include-value
#include "value.h"
//< chunk-h-include-value
//> op-enum

typedef enum {
//> op-constant
  OP_CONSTANT,

  OP_NIL,
  OP_TRUE,
  OP_FALSE,

  OP_POP,

  OP_GET_LOCAL,
  OP_SET_LOCAL,

  OP_GET_GLOBAL,

  OP_GET_UPVALUE,
  OP_SET_UPVALUE,

  OP_GET_PROPERTY,
  OP_SET_PROPERTY,

  OP_PRIVATE_PROPERTY_SET,
  OP_PRIVATE_GET_PROPERTY_NO_POP,
  OP_PRIVATE_PROPERTY_GET,

  OP_GET_PROPERTY_NO_POP,

  OP_EQUAL,
  OP_GREATER,
  OP_LESS,

  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_MOD,
  OP_POW,
  OP_BIT_AND,
  OP_BIT_OR,

  OP_NOT,

  OP_BREAK,

  OP_NEGATE,


  OP_JUMP,
  OP_JUMP_IF_FALSE,
  OP_LOOP,

  OP_CALL,
  OP_TAIL_CALL,

  OP_INVOKE,
  OP_INVOKE1,

  OP_CLOSURE,

  OP_CLOSE_UPVALUE,

  OP_RETURN,

  OP_CLASS,
  OP_METHOD,
  OP_PRIVATE_METHOD,

  OP_BUILD_LIST, 
  OP_INDEX_SUBSCR,
  OP_STORE_SUBSCR,

  OP_INDEX_SUBSCR_NO_POP,


  OP_USE,
  OP_RECENT_USE,
  OP_USE_BUILTIN,
  OP_USE_NAME,

  OP_INCREMENT,
  OP_DECREMENT,

  OP_BIT_LEFT,
  OP_BIT_RIGHT,
  OP_BIT_XOR,

  OP_ASSERT,

  OP_DEFINE_LIBRARY,
  OP_GET_LIBRARY,
  OP_SET_LIBRARY,

  OP_PRIVATE_DEFINE,
  OP_PRIVATE_GET,
  OP_PRIVATE_SET,

} OpCode;

typedef struct {

  int count;
  int capacity;

  uint8_t* code;

  int* lines;

  ValueArray constants;

} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);


#endif
