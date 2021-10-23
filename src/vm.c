#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "vm.h"
#include "natives.h"
#include "tools.h"

#include "../objects/objects.h"
#include "../libraries/library.h"

VM vm;

static void resetStack() {
  vm.stackTop = vm.stack;
  vm.frameCount = 0;
  vm.openUpvalues = NULL;
}


void runtimeError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  for (int i = vm.frameCount - 1; i >= 0; i--) {
    CallFrame* frame = &vm.frames[i];
    ObjFunction* function = frame->closure->function;

    size_t instruction = frame->ip - function->chunk.code - 1;
    fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);
    if (function->name == NULL) {
      fprintf(stderr, "%s \n", function->library->name->chars);
    } else {
      fprintf(stderr, "%s(): %s \n",
      function->name->chars,
      function->library->name->chars);
    }
  }

  resetStack();
}

void dryError(const char* error) {
  fprintf(stderr, "%s\n", error);

  for (int i = vm.frameCount - 1; i >= 0; i--) {
    CallFrame* frame = &vm.frames[i];
    ObjFunction* function = frame->closure->function;

    fprintf(stderr, " %s() in '%s' \n",
    function->name->chars,
    function->library->name->chars);
  }

  resetStack();
}

static char* resolveUse(ObjString* raw) {
  char* API = resolveLibrary(raw->chars);
  char* api_ref = API; 
  if (!API) {
    char* realpath = real(raw->chars);
    if (checkPath(realpath) == false) {
      runtimeError("Could not load \"%s\"", raw->chars);
      return NULL;
    } else {
      return realpath;
    }
  }

  return api_ref;
}

ObjClosure* compileModule(ObjLibrary* library, char* source) {
  ObjFunction* function = compile(source, library);
  pop(); //pop module.

  FREE_ARRAY(char, source, strlen(source) + 1);

  if (function == NULL) {
    return NULL;
  }

  push(OBJ_VAL(function));
  ObjClosure* closure = newClosure(function);
  pop();

  return closure;
}

void defineNative(const char* name, NativeFn function, Table* table) {
  ObjNative* native = newNative(function);
  push(OBJ_VAL(native));

  ObjString* nativeName = copyString(name, strlen(name));
  push(OBJ_VAL(nativeName));

  tableSet(table, nativeName, OBJ_VAL(native));
  pop();
  pop();
}

void defineProperty(const char* name, Value value, Table* table) {
  push(value);

  ObjString* propertyName = copyString(name, strlen(name));
  push(OBJ_VAL(propertyName));

  tableSet(table, propertyName, value);
  pop();
  pop();
}

void initVM() {

  resetStack();

  vm.objects = NULL;
  vm.markVal = true;

  vm.bytesAllocated = 0;
  vm.nextGC = 1024 * 1024;

  vm.grayCount = 0;
  vm.grayCapacity = 0;
  vm.grayStack = NULL;

  vm.recentLibrary = NULL;


  initTable(&vm.globals);
  initTable(&vm.libraries);
  initTable(&vm.strings);

  defineAllNatives();

  //
  initTable(&vm.listNativeMethods);
  initTable(&vm.numberNativeMethods);
  initTable(&vm.stringNativeMethods);
  //

  //
  initListMethods();
  initNumberMethods();
  initStringMethods();
  //

  vm.initString = NULL;
  vm.initString = copyString("init", 4);

}

void freeVM() {
  freeTable(&vm.globals);
  freeTable(&vm.strings);
  freeTable(&vm.libraries);

  //
  freeTable(&vm.listNativeMethods);
  freeTable(&vm.numberNativeMethods);
  freeTable(&vm.stringNativeMethods);
  //

  vm.initString = NULL;
  freeObjects();
}
//> push
void push(Value value) {
  *vm.stackTop = value;
  vm.stackTop++;
}

Value pop() {
  vm.stackTop--;
  return *vm.stackTop;
}

static Value peek(int distance) {
  return vm.stackTop[-1 - distance];
}

static bool call(ObjClosure* closure, int argCount) {
  if (argCount != closure->function->arity) {
    runtimeError("Expected %d arguments but got %d.", closure->function->arity, argCount);

    return false;
  }


  if (vm.frameCount == FRAMES_MAX) {
    runtimeError("Stack overflow.");
    return false;
  }

//< check-overflow
  CallFrame* frame = &vm.frames[vm.frameCount++];

  frame->closure = closure;
  frame->ip = closure->function->chunk.code;
  frame->slots = vm.stackTop - argCount - 1;
  return true;
}

static bool keepFrame(CallFrame* frame, int argCount) {
  ObjClosure* closure = AS_CLOSURE(peek(argCount));

  if (argCount != closure->function->arity) {
    runtimeError("Expected %d arguments but got %d.", closure->function->arity, argCount);
    return false;
  }

  frame->slots = vm.stackTop - argCount - 1;
  frame->closure = closure;
  frame->ip = closure->function->chunk.code;

  return true;
}

static bool callValue(Value callee, int argCount) {
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {

      case OBJ_BOUND_METHOD: {
        ObjBoundMethod* bound = AS_BOUND_METHOD(callee);
        vm.stackTop[-argCount - 1] = bound->receiver;
        return call(bound->method, argCount);
      }

      case OBJ_CLASS: {
        ObjClass* klass = AS_CLASS(callee);
        vm.stackTop[-argCount - 1] = OBJ_VAL(newInstance(klass));
        Value initializer;
        if (tableGet(&klass->methods, vm.initString, &initializer)) {
          return call(AS_CLOSURE(initializer), argCount);
        } else if (argCount != 0) {
          runtimeError("Expected 0 arguments but got %d.", argCount);
          return false;
        }

        return true;
      }

      case OBJ_CLOSURE: {
        return call(AS_CLOSURE(callee), argCount);
      }

      case OBJ_NATIVE: {
        NativeFn native = AS_NATIVE(callee);
        Value result = native(argCount, vm.stackTop - argCount);
        vm.stackTop -= argCount + 1;

        if (IS_NIL(result)) {
          return false;
        }

        push(result);
        return true;
      }
//< call-native
      default:
        break;
    }
  }
  runtimeError("Can only call functions and classes.");
  return false;
}


static bool invokeFromClass(ObjClass* klass, ObjString* name, int argCount) {
  Value method;
  if (!tableGet(&klass->methods, name, &method)) {
    runtimeError("Undefined property '%s' from '%s'.", name->chars, klass->name->chars);
    return false;
  }

  return call(AS_CLOSURE(method), argCount);
}


static bool callMethod(Value method, int argCount) {
  NativeFn native = AS_NATIVE(method);
  Value result = native(argCount, vm.stackTop - argCount - 1);

  if (IS_NIL(result)) {
    return false;
  }

  vm.stackTop -= argCount + 1;
  push(result);
  return true;
}

static bool invoke(ObjString* name, int argCount) {
  Value receiver = peek(argCount);


  if (IS_OBJ(receiver)) {
    switch (OBJ_TYPE(receiver)) {
      case OBJ_LIST: {
        Value value;
        if (tableGet(&vm.listNativeMethods, name, &value)) {
          if (IS_NATIVE(value)) {
            return callMethod(value, argCount);
          }

          //For in-lang written natives. [Dictu!]
          push(peek(0));
          for (int i = 2; i <= argCount + 1; i++) {
            vm.stackTop[-i] = peek(i);
          }

          return call(AS_CLOSURE(value), argCount + 1);
        }

        runtimeError("Undefined method '%s' from list object.", name->chars);
        return false;
      }

      case OBJ_INSTANCE: {
        ObjInstance* instance = AS_INSTANCE(receiver);

        Value value;
        if (tableGet(&instance->fields, name, &value)) {
          vm.stackTop[-argCount - 1] = value;
          return callValue(value, argCount);
        }

        return invokeFromClass(instance->klass, name, argCount);
      }

      case OBJ_STRING: {
        Value value;
        if (tableGet(&vm.stringNativeMethods, name, &value)) {
          return callMethod(value, argCount);
        }

        runtimeError("Undefined method '%s' from string objects.", name->chars);
        return false;
      }

      case OBJ_LIBRARY: {
        ObjLibrary* library = AS_LIBRARY(receiver);
        Value value;

        if (!tableGet(&library->values, name, &value)) {
          runtimeError("Undefined property '%s'.", name->chars);
          return false;
        }

        return callValue(value, argCount);
      }

      default:
        break;
    }
  } else {
    if (IS_NUMBER(receiver)) {
      Value value;
      if (tableGet(&vm.numberNativeMethods, name, &value)) {
        if (IS_NATIVE(value)) {
          return callMethod(value, argCount);
        }

        //For in-lang written natives. [Dictu!]
        push(peek(0));
        for (int i = 2; i <= argCount + 1; i++) {
          vm.stackTop[-i] = peek(i);
        }

        return call(AS_CLOSURE(value), argCount + 1);
      }

      runtimeError("Undefined method '%s' from number object.", name->chars);
      return false;
    }
  }

  runtimeError("Only instances have methods.");
  return false;
}

static bool bindMethod(ObjClass* klass, ObjString* name) {
  Value method;
  if (!tableGet(&klass->methods, name, &method)) {
    runtimeError("Undefined property '%s' from '%s'.", name->chars, klass->name->chars);
    return false;
  }

  ObjBoundMethod* bound = newBoundMethod(peek(0), AS_CLOSURE(method));
  pop();
  push(OBJ_VAL(bound));
  return true;
}

static ObjUpvalue* captureUpvalue(Value* local) {
  ObjUpvalue* prevUpvalue = NULL;
  ObjUpvalue* upvalue = vm.openUpvalues;
  while (upvalue != NULL && upvalue->location > local) {
    prevUpvalue = upvalue;
    upvalue = upvalue->next;
  }

  if (upvalue != NULL && upvalue->location == local) {
    return upvalue;
  }

  ObjUpvalue* createdUpvalue = newUpvalue(local);
  createdUpvalue->next = upvalue;

  if (prevUpvalue == NULL) {
    vm.openUpvalues = createdUpvalue;
  } else {
    prevUpvalue->next = createdUpvalue;
  }

//< insert-upvalue-in-list
  return createdUpvalue;
}

static void cleanCalls() {
  for (Value* top = vm.stackTop; top > vm.stack + 1; top--) {
    pop();
  }
}

static void closeUpvalues(Value* last) {
  while (vm.openUpvalues != NULL &&
         vm.openUpvalues->location >= last) {
    ObjUpvalue* upvalue = vm.openUpvalues;
    upvalue->closed = *upvalue->location;
    upvalue->location = &upvalue->closed;
    vm.openUpvalues = upvalue->next;
  }
}

static void defineMethod(ObjString* name) {
  Value method = peek(0);
  ObjClass* klass = AS_CLASS(peek(1));
  tableSet(&klass->methods, name, method);
  pop();
}

bool isFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {

  ObjString* b = AS_STRING(peek(0));
  ObjString* a = AS_STRING(peek(1));
//< Garbage Collection concatenate-peek

  int length = a->length + b->length;
  char* chars = ALLOCATE(char, length + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[length] = '\0';

  ObjString* result = takeString(chars, length);
//> Garbage Collection concatenate-pop
  pop();
  pop();
//< Garbage Collection concatenate-pop
  push(OBJ_VAL(result));
}


static InterpretResult run() {
  

  CallFrame* frame = &vm.frames[vm.frameCount - 1];


#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() \
    (frame->ip += 2, \
    (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

#define READ_CONSTANT() \
    (frame->closure->function->chunk.constants.values[READ_BYTE()])

#define READ_STRING() AS_STRING(READ_CONSTANT())

#define BINARY_OP(valueType, op, T) \
    do { \
      if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
        runtimeError("Operands must be numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      T b = AS_NUMBER(pop()); \
      T a = AS_NUMBER(pop()); \
      push(valueType(a op b)); \
    } while (false)

  for (;;) {
//> trace-execution
#ifdef DEBUG_TRACE_EXECUTION
//> trace-stack
    printf("          ");
    for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
      printf("[ ");
      printValue(*slot);
      printf(" ]");
    }
    printf("\n");
//< trace-stack

    disassembleInstruction(&frame->closure->function->chunk,
        (int)(frame->ip - frame->closure->function->chunk.code));
//< Closures disassemble-instruction
#endif

//< trace-execution
    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
//> op-constant
      case OP_CONSTANT: {
        Value constant = READ_CONSTANT();

        push(constant);
//< push-constant
        break;
      }
//< op-constant
//> Types of Values interpret-literals
      case OP_NIL: push(NIL_VAL); break;
      case OP_TRUE: push(BOOL_VAL(true)); break;
      case OP_FALSE: push(BOOL_VAL(false)); break;

      case OP_POP: pop(); break;

      case OP_GET_LOCAL: {
        uint8_t slot = READ_BYTE();
        push(frame->slots[slot]);
        break;
      }

      case OP_SET_LOCAL: {
        uint8_t slot = READ_BYTE();
        frame->slots[slot] = peek(0);
        break;
      }

      case OP_GET_LIBRARY: {
        ObjString* name = READ_STRING();
        Value value;
        if (!tableGet(&frame->closure->function->library->values, name, &value)) {
          runtimeError("Undefined variable '%s'.", name->chars);
          return INTERPRET_RUNTIME_ERROR;
        }
        push(value);
        break;
      }

      case OP_DEFINE_LIBRARY: {
        ObjString* name = READ_STRING();
        tableSet(&frame->closure->function->library->values, name, peek(0));
        pop();
        break;
      }

      case OP_GET_GLOBAL: {
        ObjString* name = READ_STRING();
        Value value;
        if (!tableGet(&vm.globals, name, &value)) {
          runtimeError("Undefined variable '%s'.", name->chars);
          return INTERPRET_RUNTIME_ERROR;
        }
        push(value);
        break;
      }

      case OP_SET_LIBRARY: {
        ObjString* name = READ_STRING();
        if (tableSet(&frame->closure->function->library->values, name, peek(0))) {
          tableDelete(&frame->closure->function->library->values, name); // [delete]
          runtimeError("Undefined variable '%s'.", name->chars);
          return INTERPRET_RUNTIME_ERROR;
        }
        break;
      }

      case OP_GET_UPVALUE: {
        uint8_t slot = READ_BYTE();
        push(*frame->closure->upvalues[slot]->location);
        break;
      }

      case OP_SET_UPVALUE: {
        uint8_t slot = READ_BYTE();
        *frame->closure->upvalues[slot]->location = peek(0);
        break;
      }

      case OP_GET_PROPERTY_NO_POP: {
        if (!IS_INSTANCE(peek(0))) {
          runtimeError("Only instances have properties.");
          return INTERPRET_RUNTIME_ERROR;
        }

        ObjInstance* instance = AS_INSTANCE(peek(0));
        ObjString* name = READ_STRING();
        Value val;

        if (tableGet(&instance->fields, name, &val)) {
          push(val);
          break;
        }

        if (!bindMethod(instance->klass, name)) {
          return INTERPRET_RUNTIME_ERROR;
        }

        break;
      }

      case OP_GET_PROPERTY: {
        Value receiver = peek(0);

        if (!IS_OBJ(receiver)) {
          runtimeError("Values can not have properties");
          return INTERPRET_RUNTIME_ERROR;
        }
        
        switch(OBJ_TYPE(receiver)) {
          case OBJ_INSTANCE: {
            ObjInstance* instance = AS_INSTANCE(receiver);
            ObjString* name = READ_STRING();
            
            Value value;
            if (tableGet(&instance->fields, name, &value)) {
              pop();
              push(value);
              break;
            }

            if (!bindMethod(instance->klass, name)) {
              return INTERPRET_RUNTIME_ERROR;
            }

            break;
          }

          case OBJ_LIBRARY: {
            ObjLibrary* library = AS_LIBRARY(receiver);
            ObjString* name = READ_STRING();
            
            Value value;

            if (tableGet(&library->values, name, &value)) {
              pop();
              push(value);
              break;
            }

            runtimeError("Undefined property '%s' from '%s'", name->chars, library->name->chars);
            return INTERPRET_RUNTIME_ERROR;
          }

          default:
            runtimeError("Given Type has no properties.");
            return INTERPRET_RUNTIME_ERROR;

        }

        break;
      }

      case OP_SET_PROPERTY: {
        if (!IS_OBJ(peek(1))) {
          runtimeError("Values can not have fields");
          return INTERPRET_RUNTIME_ERROR;
        }

        if (IS_INSTANCE(peek(1))) {
          ObjInstance* instance = AS_INSTANCE(peek(1));
          tableSet(&instance->fields, READ_STRING(), peek(0));
          Value value = pop();
          pop();
          push(value);
          break;
        }

        runtimeError("Given type can not have properties.");
        return INTERPRET_RUNTIME_ERROR;
      }

      case OP_EQUAL: {
        Value b = pop();
        Value a = pop();
        push(BOOL_VAL(valuesEqual(a, b)));
        break;
      }
      case OP_GREATER:  BINARY_OP(BOOL_VAL, >, double); break;
      case OP_LESS:     BINARY_OP(BOOL_VAL, <, double); break;
      case OP_ADD: {
        if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
          concatenate();
        } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
          double b = AS_NUMBER(pop());
          double a = AS_NUMBER(pop());
          push(NUMBER_VAL(a + b));
        } else {
          runtimeError("Operands must be two numbers or two strings.");
          return INTERPRET_RUNTIME_ERROR;
        }
        break;
      }

      case OP_BIT_AND: BINARY_OP(NUMBER_VAL, &, int); break;
      case OP_BIT_OR: BINARY_OP(NUMBER_VAL, |, int); break;
      case OP_MOD: {
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
          runtimeError("Operands must be numbers.");
          return INTERPRET_RUNTIME_ERROR;
        }

        double b = AS_NUMBER(pop());
        double a = AS_NUMBER(pop());

        push(NUMBER_VAL(fmod(a,b)));
        break;
      }

      case OP_POW: {
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
          runtimeError("Operands must be numbers.");
          return INTERPRET_RUNTIME_ERROR;
        }

        double b = AS_NUMBER(pop());
        double a = AS_NUMBER(pop());

        push(NUMBER_VAL(powf(a,b)));
        break;
      }

      case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -, double); break;
      case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *, double); break;
      case OP_DIVIDE: {
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
          runtimeError("Operands must be numbers.");
          return INTERPRET_RUNTIME_ERROR;
        }

        double b = AS_NUMBER(pop());
        double a = AS_NUMBER(pop());

        if (a == 0 || b == 0) {
          runtimeError("Can not divide by 0.");
          return INTERPRET_RUNTIME_ERROR;
        }

        push(NUMBER_VAL(a / b));
        break;
      }
      
      case OP_NOT:
        push(BOOL_VAL(isFalsey(pop())));
        break;
      case OP_NEGATE:
        if (!IS_NUMBER(peek(0))) {
          runtimeError("Operand must be a number.");
          return INTERPRET_RUNTIME_ERROR;
        }
        push(NUMBER_VAL(-AS_NUMBER(pop())));
        break;

      case OP_PRINT: {
        printValue(pop());
        printf("\n");
        break;
      }

      case OP_JUMP: {
        uint16_t offset = READ_SHORT();
        frame->ip += offset;
        break;
      }

      case OP_JUMP_IF_FALSE: {
        uint16_t offset = READ_SHORT();

        if (isFalsey(peek(0))) frame->ip += offset;
        break;
      }

      case OP_LOOP: {
        uint16_t offset = READ_SHORT();

        frame->ip -= offset;
        break;
      }

      case OP_CALL: {
        int argCount = READ_BYTE();

        if (!callValue(peek(argCount), argCount)) {
          return INTERPRET_RUNTIME_ERROR;
        }

        frame = &vm.frames[vm.frameCount - 1];
        break;
      }

      case OP_TAIL_CALL: {
        int argCount = READ_BYTE();

        if (frame != &vm.frames[vm.frameCount]) {
          if (!callValue(peek(argCount), argCount)) {
            return INTERPRET_RUNTIME_ERROR;
          }

          frame = &vm.frames[vm.frameCount - 1];
          break;
        }

        if (!keepFrame(frame, argCount)) {
          return INTERPRET_RUNTIME_ERROR;
        }

        // Awkward stack cleaner hack.
        Value result = pop();
        cleanCalls();

        push(result);
        frame = &vm.frames[vm.frameCount - 1];
        break;
      }

      case OP_INVOKE: {
        ObjString* method = READ_STRING();
        int argCount = READ_BYTE();

        if (!invoke(method, argCount)) {
          return INTERPRET_RUNTIME_ERROR;
        }

        frame = &vm.frames[vm.frameCount - 1];
        break;
      }

      case OP_CLOSURE: {
        ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
        ObjClosure* closure = newClosure(function);
        push(OBJ_VAL(closure));
        for (int i = 0; i < closure->upvalueCount; i++) {
          uint8_t isLocal = READ_BYTE();
          uint8_t index = READ_BYTE();
          if (isLocal) {
            closure->upvalues[i] =
                captureUpvalue(frame->slots + index);
          } else {
            closure->upvalues[i] = frame->closure->upvalues[index];
          }
        }
        break;
      }

      case OP_CLOSE_UPVALUE:
        closeUpvalues(vm.stackTop - 1);
        pop();
        break;


      case OP_RETURN: {
        Value result = pop();
        closeUpvalues(frame->slots);

        if (--vm.frameCount == 0) {
          pop();
          return INTERPRET_OK;
        }

        vm.stackTop = frame->slots;

        push(result);
        frame = &vm.frames[vm.frameCount - 1];
        break;
      }

      case OP_CLASS:
        push(OBJ_VAL(newClass(READ_STRING())));
        break;

      case OP_METHOD:
        defineMethod(READ_STRING());
        break;

      case OP_BREAK:
        break;

      case OP_BUILD_LIST: {
        ObjList* list = newList();
        uint8_t itemCount = READ_BYTE();

        push(OBJ_VAL(list));

        for (int i = itemCount; i > 0; i--) {
          appendToList(list, peek(i));
        }
        
        vm.stackTop -= itemCount + 1; 
        push(OBJ_VAL(list));
        break;
      }

      case OP_INDEX_SUBSCR_NO_POP: {
        Value val;
        Value indexVal = peek(0);
        Value subscrVal = peek(1);

        if (!IS_LIST(subscrVal)) {
          runtimeError("Invalid type for subscripting.");
          return INTERPRET_RUNTIME_ERROR;
        }

        if (!IS_NUMBER(indexVal)) {
          printValue(indexVal);
          runtimeError("Index must be a number.");
          return INTERPRET_RUNTIME_ERROR;
        }
        
        int index = AS_NUMBER(indexVal);
        ObjList* list = AS_LIST(subscrVal);

        if (!isValidListIndex(list, index)) {
          runtimeError("List index out of range.");
          return INTERPRET_RUNTIME_ERROR;
        }

        val = indexFromList(list, index);
        push(val);
        break;
      }

      case OP_INDEX_SUBSCR: {
        Value indexVal = pop();
        Value objVal = pop();
        Value result;

        if (!IS_OBJ(objVal)) {
          runtimeError("Invalid type for subscripting.");
          return INTERPRET_RUNTIME_ERROR;
        }

        if (!IS_NUMBER(indexVal)) {
          runtimeError("Index must be a number.");
          return INTERPRET_RUNTIME_ERROR;
        }
        
        int index = AS_NUMBER(indexVal);

        switch(OBJ_TYPE(objVal)) {
          case OBJ_LIST: {
            ObjList* list = AS_LIST(objVal);

            if (!isValidListIndex(list, index)) {
              runtimeError("List index out of range.");
              return INTERPRET_RUNTIME_ERROR;
            }

            result = indexFromList(list, index);
            push(result);
            break;
          }

          case OBJ_STRING: {
            ObjString* string = AS_STRING(objVal);

            if (!isValidStringIndex(string, index)) {
              runtimeError("String index out of range.");
              return INTERPRET_RUNTIME_ERROR;
            }

            result = indexFromString(string, index);
            push(result);
            break;
          }

          default:
            runtimeError("Type not subscriptable.");
            return INTERPRET_RUNTIME_ERROR;
        }

        break;
      }

      case OP_STORE_SUBSCR: {
        Value item = pop();
        Value indexVal = pop();
        Value listVal = pop();

        if (!IS_LIST(listVal)) {
          runtimeError("Cannot store value in a non-list.");
          return INTERPRET_RUNTIME_ERROR;
        }
        ObjList* list = AS_LIST(listVal);

        if (!IS_NUMBER(indexVal)) {
          runtimeError("List index must be a number.");
          return INTERPRET_RUNTIME_ERROR;
        }
        int index = AS_NUMBER(indexVal);

        if (!isValidListIndex(list, index)) {
          runtimeError("Index out of range");
          return INTERPRET_RUNTIME_ERROR;
        }

        storeToList(list, index, item);
        push(item);
        break;
      }

      case OP_USE_NAME: {
        push(OBJ_VAL(vm.recentLibrary));
        break;
      }

      case OP_USE_BUILTIN: {
        int index = READ_BYTE();
        ObjString* name = READ_STRING();
        Value libVal;

        if (tableGet(&vm.libraries, name, &libVal)) {
          push(libVal);
          break;
        }

        ObjLibrary* library = importLibrary(index);

        push(OBJ_VAL(library));
        break;
      }

      case OP_INCREMENT: {
        if (!IS_NUMBER(peek(0))) {
          runtimeError("Operand must be number.");
          return INTERPRET_RUNTIME_ERROR;
        }

        push(NUMBER_VAL(AS_NUMBER(pop()) + 1));
        break;
      }

      case OP_DECREMENT: {
        if (!IS_NUMBER(peek(0))) {
          runtimeError("Operand must be number.");
          return INTERPRET_RUNTIME_ERROR;
        }

        push(NUMBER_VAL(AS_NUMBER(pop()) - 1));
        break;
      }

      case OP_USE: {
        ObjString* name = READ_STRING();
        Value libValue;

        if (tableGet(&vm.libraries, name, &libValue)) {
          vm.recentLibrary = AS_LIBRARY(libValue);
          push(NIL_VAL);
          break;
        }
        
        char* api_ref = resolveUse(name);
        if (!api_ref) return INTERPRET_RUNTIME_ERROR;

        char* source = readFile(api_ref);

        push(OBJ_VAL(name));
        ObjLibrary* library = newLibrary(name);
        vm.recentLibrary = library;
        pop();
        
        FREE_ARRAY(char, api_ref, strlen(api_ref) + 1);

        push(OBJ_VAL(library));
        ObjClosure* closure = compileModule(library, source);
        if (!closure) return INTERPRET_COMPILE_ERROR;

        push(OBJ_VAL(closure));
        call(closure, 0);

        frame = &vm.frames[vm.frameCount - 1];
        break;
      }

      case OP_RECENT_USE: {
        vm.recentLibrary = frame->closure->function->library;
        break;
      }
    }
  }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP

}

InterpretResult interpret(const char* source, char* libName) {
  ObjString* name = copyString(libName, strlen(libName));
  push(OBJ_VAL(name));
  ObjLibrary* library = newLibrary(name);
  pop();

  ObjFunction* function = compile(source, library);
  if (function == NULL) return INTERPRET_COMPILE_ERROR;

  push(OBJ_VAL(function));
  ObjClosure* closure = newClosure(function);
  pop();
  push(OBJ_VAL(closure));
  callValue(OBJ_VAL(closure), 0);
  return run();
}