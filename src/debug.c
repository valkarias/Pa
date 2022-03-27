//> Chunks of Bytecode debug-c
#include <stdio.h>

#include "debug.h"
//> Closures debug-include-object
#include "object.h"
//< Closures debug-include-object
//> debug-include-value
#include "value.h"
//< debug-include-value

void disassembleChunk(Chunk* chunk, const char* name) {
  printf("== %s ==\n", name);
  
  for (int offset = 0; offset < chunk->count;) {
    offset = disassembleInstruction(chunk, offset);
  }
}
//> constant-instruction
static int constantInstruction(const char* name, Chunk* chunk,
                               int offset) {
  uint8_t constant = chunk->code[offset + 1];
  printf("%-16s %4d '", name, constant);
  printValue(chunk->constants.values[constant]);
  printf("'\n");
//> return-after-operand
  return offset + 2;
//< return-after-operand
}

static int invokeInstruction(const char* name, Chunk* chunk,
                                int offset) {
  uint8_t constant = chunk->code[offset + 1];
  uint8_t argCount = chunk->code[offset + 2];
  printf("%-16s (%d args) %4d '", name, argCount, constant);
  printValue(chunk->constants.values[constant]);
  printf("'\n");
  return offset + 3;
}

static int simpleInstruction(const char* name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

static int byteInstruction(const char* name, Chunk* chunk,
                           int offset) {
  uint8_t slot = chunk->code[offset + 1];
  printf("%-16s %4d\n", name, slot);
  return offset + 2; // [debug]
}

static int jumpInstruction(const char* name, int sign,
                           Chunk* chunk, int offset) {
  uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
  jump |= chunk->code[offset + 2];
  printf("%-16s %4d -> %d\n", name, offset,
         offset + 3 + sign * jump);
  return offset + 3;
}

int disassembleInstruction(Chunk* chunk, int offset) {
  printf("%04d ", offset);
//> show-location
  if (offset > 0 &&
      chunk->lines[offset] == chunk->lines[offset - 1]) {
    printf("   | ");
  } else {
    printf("%4d ", chunk->lines[offset]);
  }
//< show-location
  
  uint8_t instruction = chunk->code[offset];
  switch (instruction) {

    case OP_CONSTANT:
      return constantInstruction("OP_CONSTANT", chunk, offset);

    case OP_NIL:
      return simpleInstruction("OP_NIL", offset);
    case OP_BREAK:
      return simpleInstruction("OP_BREAK", offset);
    case OP_TRUE:
      return simpleInstruction("OP_TRUE", offset);
    case OP_FALSE:
      return simpleInstruction("OP_FALSE", offset);

    case OP_POP:
      return simpleInstruction("OP_POP", offset);
    case OP_STORE_SUBSCR:
      return simpleInstruction("OP_STORE_SUBSCRIPT", offset);
    case OP_INDEX_SUBSCR:
      return simpleInstruction("OP_INDEX_SUBSCRIPT", offset);

    case OP_INDEX_SUBSCR_NO_POP:
      return simpleInstruction("OP_INDEX_SUBSCR_NO_POP", offset);

    case OP_USE_NAME:
      return simpleInstruction("OP_USE_NAME", offset);


    case OP_BUILD_LIST:
      return byteInstruction("OP_BUILD_LIST", chunk, offset);

    case OP_GET_LOCAL:
      return byteInstruction("OP_GET_LOCAL", chunk, offset);
    case OP_SET_LOCAL:
      return byteInstruction("OP_SET_LOCAL", chunk, offset);

    case OP_GET_LIBRARY:
      return constantInstruction("OP_GET_LIBRARY", chunk, offset);
    case OP_PRIVATE_GET:
      return constantInstruction("OP_PRIVATE_GET", chunk, offset);

    case OP_GET_GLOBAL:
      return constantInstruction("OP_GET_GLOBAL", chunk, offset);

    case OP_DEFINE_LIBRARY:
      return constantInstruction("OP_DEFINE_LIBRARY", chunk, offset);
    case OP_PRIVATE_DEFINE:
      return constantInstruction("OP_PRIVATE_DEFINE", chunk, offset);

    case OP_USE:
      return constantInstruction("OP_USE", chunk, offset);
    
    case OP_RECENT_USE:
      return simpleInstruction("OP_RECENT_USE", offset);

    case OP_SET_LIBRARY:
      return constantInstruction("OP_SET_LIBRARY", chunk, offset);
    case OP_PRIVATE_SET:
      return constantInstruction("OP_PRIVATE_SET", chunk, offset);

    case OP_ASSERT:
      return constantInstruction("OP_ASSERT", chunk, offset);

    case OP_GET_UPVALUE:
      return byteInstruction("OP_GET_UPVALUE", chunk, offset);
    case OP_SET_UPVALUE:
      return byteInstruction("OP_SET_UPVALUE", chunk, offset);

    case OP_GET_PROPERTY:
      return constantInstruction("OP_GET_PROPERTY", chunk, offset);
    case OP_GET_PROPERTY_NO_POP:
      return constantInstruction("OP_GET_PROPERTY_NO_POP", chunk, offset);
    case OP_PRIVATE_GET_PROPERTY_NO_POP:
      return constantInstruction("OP_PRIVATE_GET_PROPERTY_NO_POP", chunk, offset);
    case OP_SET_PROPERTY:
      return constantInstruction("OP_SET_PROPERTY", chunk, offset);

    case OP_PRIVATE_PROPERTY_GET:
      return constantInstruction("OP_PRIVATE_PROPERTY_GET", chunk, offset);
    case OP_PRIVATE_PROPERTY_SET:
      return constantInstruction("OP_PRIVATE_PROPERTY_SET", chunk, offset);

    case OP_USE_BUILTIN:
      return simpleInstruction("OP_USE_BUILTIN", offset);

    case OP_EQUAL:
      return simpleInstruction("OP_EQUAL", offset);
    case OP_GREATER:
      return simpleInstruction("OP_GREATER", offset);
    case OP_LESS:
      return simpleInstruction("OP_LESS", offset);

    case OP_INCREMENT:
      return simpleInstruction("OP_INCREMENT", offset);
    case OP_DECREMENT:
      return simpleInstruction("OP_DECREMENT", offset);

    case OP_ADD:
      return simpleInstruction("OP_ADD", offset);
    case OP_BIT_AND:
      return simpleInstruction("OP_BIT_AND", offset);
    case OP_BIT_OR:
      return simpleInstruction("OP_BIT_OR", offset);
    case OP_BIT_XOR:
      return simpleInstruction("OP_BIT_XOR", offset);
    case OP_BIT_LEFT:
      return simpleInstruction("OP_BIT_LEFT", offset);
    case OP_BIT_RIGHT:
      return simpleInstruction("OP_BIT_RIGHT", offset);
    case OP_SUBTRACT:
      return simpleInstruction("OP_SUBTRACT", offset);
    case OP_MULTIPLY:
      return simpleInstruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:
      return simpleInstruction("OP_DIVIDE", offset);
//> Types of Values disassemble-not
    case OP_NOT:
      return simpleInstruction("OP_NOT", offset);

    case OP_NEGATE:
      return simpleInstruction("OP_NEGATE", offset);

    case OP_JUMP:
      return jumpInstruction("OP_JUMP", 1, chunk, offset);
    case OP_JUMP_IF_FALSE:
      return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);

    case OP_LOOP:
      return jumpInstruction("OP_LOOP", -1, chunk, offset);

    case OP_CALL:
      return byteInstruction("OP_CALL", chunk, offset);
    case OP_TAIL_CALL:
      return byteInstruction("OP_TAIL_CALL", chunk, offset);

    case OP_INVOKE:
      return invokeInstruction("OP_INVOKE", chunk, offset);
    case OP_INVOKE1:
      return invokeInstruction("OP_INVOKE1", chunk, offset);  


    case OP_CLOSURE: {
      offset++;
      uint8_t constant = chunk->code[offset++];
      printf("%-16s %4d ", "OP_CLOSURE", constant);
      printValue(chunk->constants.values[constant]);
      printf("\n");


      ObjFunction* function = AS_FUNCTION(
          chunk->constants.values[constant]);
      for (int j = 0; j < function->upvalueCount; j++) {
        int isLocal = chunk->code[offset++];
        int index = chunk->code[offset++];
        printf("%04d      |                     %s %d\n",
               offset - 2, isLocal ? "local" : "upvalue", index);
      }
      
      return offset;
    }

    case OP_CLOSE_UPVALUE:
      return simpleInstruction("OP_CLOSE_UPVALUE", offset);

    case OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);

    case OP_CLASS:
      return constantInstruction("OP_CLASS", chunk, offset);


    case OP_METHOD:
      return constantInstruction("OP_METHOD", chunk, offset);
    case OP_PRIVATE_METHOD:
      return constantInstruction("OP_PRIVATE_METHOD", chunk, offset);

    default:
      printf("Unknown opcode %d\n", instruction);
      return offset + 1;
  }
}