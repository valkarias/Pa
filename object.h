//> Strings object-h
#ifndef pcrap_object_h
#define pcrap_object_h

#include "common.h"

#include "chunk.h"

#include "table.h"

#include "value.h"
//> obj-type-macro

#define OBJ_TYPE(value)        (AS_OBJ(value)->type)

#define IS_BOUND_METHOD(value) isObjType(value, OBJ_BOUND_METHOD)

#define IS_CLASS(value)        isObjType(value, OBJ_CLASS)

#define IS_CLOSURE(value)      isObjType(value, OBJ_CLOSURE)

#define IS_FUNCTION(value)     isObjType(value, OBJ_FUNCTION)

#define IS_INSTANCE(value)     isObjType(value, OBJ_INSTANCE)

#define IS_NATIVE(value)       isObjType(value, OBJ_NATIVE)

#define IS_STRING(value)       isObjType(value, OBJ_STRING)

#define IS_LIST(value)       isObjType(value, OBJ_LIST)
#define IS_LIBRARY(value)    isObjType(value, OBJ_LIBRARY)



#define AS_LIBRARY(value)       ((ObjLibrary*)AS_OBJ(value))
#define AS_BOUND_METHOD(value) ((ObjBoundMethod*)AS_OBJ(value))
#define AS_CLASS(value)        ((ObjClass*)AS_OBJ(value))
#define AS_LIST(value)        ((ObjList*)AS_OBJ(value))

#define AS_CLOSURE(value)      ((ObjClosure*)AS_OBJ(value))

#define AS_FUNCTION(value)     ((ObjFunction*)AS_OBJ(value))

#define AS_INSTANCE(value)     ((ObjInstance*)AS_OBJ(value))

#define AS_NATIVE(value) \
    (((ObjNative*)AS_OBJ(value))->function)

#define AS_STRING(value)       ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)      (((ObjString*)AS_OBJ(value))->chars)

typedef enum {

  OBJ_BOUND_METHOD,

  OBJ_CLASS,

  OBJ_CLOSURE,

  OBJ_FUNCTION,

  OBJ_INSTANCE,

  OBJ_NATIVE,

  OBJ_STRING,

  OBJ_UPVALUE,

  OBJ_LIST,

  OBJ_LIBRARY,
} ObjType;


struct Obj {
  ObjType type;
  bool mark;
  struct Obj* next;
};

typedef struct {
  Obj obj;

  ObjString* name;
  Table values;

} ObjLibrary;

typedef enum {
  TYPE_FUNCTION,

  TYPE_INITIALIZER,
  TYPE_METHOD,
  TYPE_SCRIPT
} FunctionType;

typedef enum {
  FUNCTION_PROTECTED,
  FUNCTION_NOT_PROTECTED,

} FunctionProtection;

typedef struct {
  Obj obj;
  int arity;

  int upvalueCount;

  Chunk chunk;
  ObjString* name;

  ObjLibrary* library;

  FunctionType type;
  FunctionProtection protection;
} ObjFunction;


typedef Value (*NativeFn)(int argCount, Value* args);

typedef struct {
  Obj obj;
  NativeFn function;
} ObjNative;


struct ObjString {
  Obj obj;
  int length;
  char* chars;
  uint32_t hash;

};

typedef struct ObjUpvalue {
  Obj obj;
  Value* location;

  Value closed;

  struct ObjUpvalue* next;
//< next-field
} ObjUpvalue;

typedef struct {
  Obj obj;
  ObjFunction* function;
//> upvalue-fields
  ObjUpvalue** upvalues;
  int upvalueCount;
//< upvalue-fields
} ObjClosure;


typedef struct {
  Obj obj;
  ObjString* name;

  Table methods;

} ObjClass;

typedef struct {
    Obj obj;
    int count;
    int capacity;
    Value* items;
} ObjList;

typedef struct {
  Obj obj;
  ObjClass* klass;
  Table fields;
} ObjInstance;

typedef struct {
  Obj obj;
  Value receiver;
  ObjClosure* method;
} ObjBoundMethod;



ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method);
ObjClass* newClass(ObjString* name);

ObjLibrary* newLibrary(ObjString* name);

ObjClosure* newClosure(ObjFunction* function);

ObjFunction* newFunction(ObjLibrary* library, FunctionProtection protection, FunctionType type);

ObjInstance* newInstance(ObjClass* klass);

ObjList* newList();
void appendToList(ObjList* list, Value value);
Value indexFromList(ObjList* list, int index);
bool isValidListIndex(ObjList* list, int index);
void deleteFromList(ObjList* list, int index);
void storeToList(ObjList* list, int index, Value value);
void clearList(ObjList* list);


ObjNative* newNative(NativeFn function);

ObjString* takeString(char* chars, int length);
ObjString* copyString(const char* chars, int length);

bool isValidStringIndex(ObjString* string, int index);
Value indexFromString(ObjString* string, int index);

ObjUpvalue* newUpvalue(Value* slot);

void printObject(Value value);

static inline bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

//< is-obj-type
#endif
