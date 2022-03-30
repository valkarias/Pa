#ifndef Pa_vm_h
#define Pa_vm_h


#include "object.h"

#include "table.h"

#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {

  ObjClosure* closure;

  uint8_t* ip;
  Value* slots;
} CallFrame;

typedef struct {

  CallFrame frames[FRAMES_MAX];
  int frameCount;
  bool markVal;
  

  Value stack[STACK_MAX];
  Value* stackTop;

  Table libraries;
  ObjLibrary* recentLibrary;

  //
  Table listNativeMethods;
  Table numberNativeMethods;
  Table stringNativeMethods;
  //

  Table globals;
  Table strings;
  ObjString* initString;

  ObjUpvalue* openUpvalues;

  size_t bytesAllocated;
  size_t nextGC;

  Obj* objects;

  int grayCount;
  int grayCapacity;
  Obj** grayStack;

} VM;

//> interpret-result
typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;


extern VM vm;

void initVM();
void freeVM();


InterpretResult interpret(const char* source, char* libName);
void defineNative(const char* name, NativeFn function, Table* table);
void defineProperty(const char* name, Value value, Table* table);
void runtimeError(const char* format, ...);
void info(const char* extra, ...);
void push(Value value);
Value pop();

bool isFalsey(Value value);

bool callValue(Value callee, int argCount);
#endif
