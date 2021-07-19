//> Strings object-c
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

#include "memory.h"
#include "object.h"
//> Hash Tables object-include-table
#include "table.h"
//< Hash Tables object-include-table
#include "value.h"
#include "vm.h"
//> allocate-obj

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType)


static Obj* allocateObject(size_t size, ObjType type) {
  Obj* object = (Obj*)reallocate(NULL, 0, size);
  object->type = type;
//> Garbage Collection init-is-marked
  object->mark = !vm.markVal;
//< Garbage Collection init-is-marked
//> add-to-list
  
  object->next = vm.objects;
  vm.objects = object;
//< add-to-list
//> Garbage Collection debug-log-allocate

#ifdef DEBUG_LOG_GC
  printf(" %p allocate %zu for %d\n", (void*)object, size, type);
#endif

//< Garbage Collection debug-log-allocate
  return object;
}
//< allocate-object
//> Methods and Initializers new-bound-method
ObjBoundMethod* newBoundMethod(Value receiver,
                               ObjClosure* method) {
  ObjBoundMethod* bound = ALLOCATE_OBJ(ObjBoundMethod,
                                       OBJ_BOUND_METHOD);
  bound->receiver = receiver;
  bound->method = method;
  return bound;
}
//< Methods and Initializers new-bound-method
//> Classes and Instances new-class
ObjClass* newClass(ObjString* name) {
  ObjClass* klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
  klass->name = name; // [klass]
//> Methods and Initializers init-methods
  initTable(&klass->methods);
//< Methods and Initializers init-methods
  return klass;
}
//< Classes and Instances new-class
//> Closures new-closure
ObjClosure* newClosure(ObjFunction* function) {
//> allocate-upvalue-array
  ObjUpvalue** upvalues = ALLOCATE(ObjUpvalue*,
                                   function->upvalueCount);
  for (int i = 0; i < function->upvalueCount; i++) {
    upvalues[i] = NULL;
  }

//< allocate-upvalue-array
  ObjClosure* closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
  closure->function = function;
//> init-upvalue-fields
  closure->upvalues = upvalues;
  closure->upvalueCount = function->upvalueCount;
//< init-upvalue-fields
  return closure;
}

ObjLibrary* newLibrary(ObjString* name) {
  
  Value libVal;
  if (tableGet(&vm.libraries, name, &libVal)) {
    return AS_LIBRARY(libVal);
  }

  ObjLibrary* library = ALLOCATE_OBJ(ObjLibrary, OBJ_LIBRARY);
  initTable(&library->values);
  library->name = name;


  // garbage collectors are a nightmare.
  push(OBJ_VAL(library));
  ObjString* __name__ = copyString("__name__", 8); 
  push(OBJ_VAL(__name__));
  tableSet(&library->values, __name__, OBJ_VAL(name));
  //

  tableSet(&vm.libraries, name, OBJ_VAL(library));

  pop();
  pop();
  return library;
}

ObjFunction* newFunction(ObjLibrary* library, FunctionProtection protection, FunctionType type) {
  ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
  function->arity = 0;
  function->upvalueCount = 0;

  function->library = library;
  function->type = type;
  function->protection = protection;

  function->name = NULL;
  initChunk(&function->chunk);
  return function;
}

ObjInstance* newInstance(ObjClass* klass) {
  ObjInstance* instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
  instance->klass = klass;
  initTable(&instance->fields);
  return instance;
}

ObjNative* newNative(NativeFn function) {
  ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
  native->function = function;
  return native;
}

ObjList* newList() {
  ObjList* list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
  initValueArray(&list->items);
  return list;
}

void appendToList(ObjList* list, Value value) {
  writeValueArray(&list->items, value);
}

Value indexFromList(ObjList* list, int index) {
  return list->items.values[index];
}

void storeToList(ObjList* list, int index, Value value) {
  list->items.values[index] = value;
}

bool isValidListIndex(ObjList* list, int index) {
  if (index < 0 || index > list->items.count - 1) {
    return false;
  }
  return true;
}

void deleteFromList(ObjList* list, int index) {
  for (int i = index; i < list->items.count - 1; i++) {
    list->items.values[i] = list->items.values[i + 1];
  }

  list->items.count--;
}

void clearList(ObjList* list) {
  const int count = list->items.count;

  for (int i = 0; i < count; i++) {
    deleteFromList(list, i);
  }
}

static ObjString* allocateString(char* chars, int length, uint32_t hash) {
//< Hash Tables allocate-string
  ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  string->length = length;
  string->chars = chars;
  string->hash = hash;

  push(OBJ_VAL(string));
  tableSet(&vm.strings, string, NIL_VAL);
  pop();

  return string;
}

static uint32_t blend(uint32_t h) {
  h *=  0xcc9e2d51;
  h = (h << 15) | (h >> 17);
  h *= 0x1b873593;
  return h;
}

///// MurMur hash algorithm.

/* Copyright 2013, Sébastien Paolacci.
All rights reserved. Check NOTICE.TXT for more.
*/
static uint32_t hashString(const char* key, int length) {
  uint32_t h1 = 0x0;
  uint32_t k;
  for (size_t i = length >> 2; i; i--) {
    memcpy(&k, key, sizeof(uint32_t));
    key += sizeof(uint32_t);
    h1 ^= blend(k);
    h1 = (h1 << 13) | (h1 >> 19);
    h1 = h1 * 5 + 0xe6546b64;
  }

  k = 0;
  for (size_t i = length & 3; i; i--) {
    k <<= 8;
    k |= key[i - 1];
  }

  h1 ^= blend(k);

  h1 ^= length;
  h1 ^= h1 >> 16;
  h1 *= 0x85ebca6b;
  h1 ^= h1 >> 13;
  h1 *= 0xc2b2ae35;
  h1 ^= h1 >> 16;

  return h1;
}
//< Hash Tables hash-string
//> take-string
ObjString* takeString(char* chars, int length) {
  uint32_t hash = hashString(chars, length);

  ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
  if (interned != NULL) {
    FREE_ARRAY(char, chars, length + 1);
    return interned;
  }

  return allocateString(chars, length, hash);
//< Hash Tables take-string-hash
}
//< take-string
ObjString* copyString(const char* chars, int length) {

  uint32_t hash = hashString(chars, length);
  ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
  if (interned != NULL) return interned;


  char* heapChars = ALLOCATE(char, length + 1);
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';

  return allocateString(heapChars, length, hash);
}

bool isValidStringIndex(ObjString* string, int index) {
  if (index < 0 || index > string->length - 1) {
    return false;
  }

  return true;
}

Value indexFromString(ObjString* string, int index) {
  return OBJ_VAL(copyString(&string->chars[index], 1));
}

//> Closures new-upvalue
ObjUpvalue* newUpvalue(Value* slot) {
  ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
//> init-closed
  upvalue->closed = NIL_VAL;
//< init-closed
  upvalue->location = slot;
//> init-next
  upvalue->next = NULL;
//< init-next
  return upvalue;
}
//< Closures new-upvalue
//> Calls and Functions print-function-helper
static void printFunction(ObjFunction* function) {
  if (function->name == NULL) {
    printf("<script>");
    return;
  }
//< print-script
  printf("<fn %s>", function->name->chars);
}

void* generateType (char* type) {
  size_t len = strlen(type) + 1;

  char* c = ALLOCATE(char, len);
  memcpy(c, type, len);
  c[len] = '\0';
  
  return c;
}

char* typeObject(Value value) {
  switch (OBJ_TYPE(value)) {
    case OBJ_BOUND_METHOD:
    case OBJ_CLOSURE:
      return generateType("function");
    
    case OBJ_CLASS:
      return generateType("class");
    
    case OBJ_LIBRARY:
      return generateType("library");

    case OBJ_LIST:
      return generateType("list");

    case OBJ_INSTANCE: {
      char* c = malloc(9 + AS_INSTANCE(value)->klass->name->length);
      c = strcat("instance ", AS_INSTANCE(value)->klass->name->chars);
      
      return generateType(c);
    }

    case OBJ_NATIVE:
      return generateType("native");

    case OBJ_STRING:
      return generateType("string");
    
    case OBJ_UPVALUE:
      return generateType("upvalue");
  }

  return generateType("unknown");
}

void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
    case OBJ_BOUND_METHOD:
      printFunction(AS_BOUND_METHOD(value)->method->function);
      break;

    case OBJ_CLASS:
      printf("%s", AS_CLASS(value)->name->chars);
      break;

    case OBJ_LIBRARY: {
      ObjLibrary* library = AS_LIBRARY(value);
      if (library->name == NULL) {
        printf("<library>");
        break;
      }

      printf("<library %s>", library->name->chars);
      break;
    }

    case OBJ_LIST: {
      ObjList* list = AS_LIST(value);
      printf("[");
      for (int i = 0; i < list->items.count; i++) {
        printf(" ");
        printValue(list->items.values[i]);
        printf(" ");
      }

      printf("]");
      break;
    }
//< Classes and Instances print-class
//> Closures print-closure
    case OBJ_CLOSURE:
      printFunction(AS_CLOSURE(value)->function);
      break;
//< Closures print-closure
//> Calls and Functions print-function
    case OBJ_FUNCTION:
      printFunction(AS_FUNCTION(value));
      break;
//< Calls and Functions print-function
//> Classes and Instances print-instance
    case OBJ_INSTANCE:
      printf("%s instance",
             AS_INSTANCE(value)->klass->name->chars);
      break;
//< Classes and Instances print-instance
//> Calls and Functions print-native
    case OBJ_NATIVE:
      printf("<native fn>");
      break;
//< Calls and Functions print-native
    case OBJ_STRING:
      printf("%s", AS_CSTRING(value));
      break;
//> Closures print-upvalue
    case OBJ_UPVALUE:
      printf("upvalue");
      break;
//< Closures print-upvalue
  }
}
//< print-object
