//> Chunks of Bytecode value-c
#include <stdio.h>
//> Strings value-include-string
#include <string.h>
//< Strings value-include-string

//> Strings value-include-object
#include "object.h"
//< Strings value-include-object
#include "memory.h"
#include "value.h"

void initValueArray(ValueArray* array) {
  array->values = NULL;
  array->capacity = 0;
  array->count = 0;
}
//> write-value-array
void writeValueArray(ValueArray* array, Value value) {
  if (array->capacity < array->count + 1) {
    int oldCapacity = array->capacity;
    array->capacity = GROW_CAPACITY(oldCapacity);
    array->values = GROW_ARRAY(Value, array->values,
                               oldCapacity, array->capacity);
  }
  
  array->values[array->count] = value;
  array->count++;
}
//< write-value-array
//> free-value-array
void freeValueArray(ValueArray* array) {
  FREE_ARRAY(Value, array->values, array->capacity);
  initValueArray(array);
}

char* stringValue(Value value) {
  if (IS_BOOL(value)) {
    char* str = AS_BOOL(value) ? "true" : "false";
    char* valueStr = malloc(sizeof(char) * (strlen(str) + 1));
    snprintf(valueStr, strlen(str) + 1, "%s", str);
    return valueStr;

  } else if (IS_NIL(value)) {
    //4 for 'none' + 1 for '\0'
    char* valueStr = malloc(sizeof(char) * 5);
    snprintf(valueStr, 5, "%s", "none");
    return valueStr;

  } else if (IS_NUMBER(value)) {
    double num = AS_NUMBER(value);
    int length = snprintf(NULL, 0, "%.15g", num) + 1;
    char* valueStr = malloc(sizeof(char) * length);
    snprintf(valueStr, length, "%.15g", num);
    return valueStr;

  } else if (IS_OBJ(value)) {
    return objectString(value);
  }
}

char* typeValue(Value value) {
  if (IS_BOOL(value)) {
    return generateType("bool");

  } else if (IS_NUMBER(value)) {
    return generateType("number");
  } else if (IS_NIL(value)) {
    return generateType("none");
  } else if (IS_OBJ(value)) {
    return typeObject(value);
  }

  return generateType("unknown");
}

void printValue(Value value) {
//> Optimization print-value
#ifdef NAN_BOXING
  if (IS_BOOL(value)) {
    printf(AS_BOOL(value) ? "true" : "false");
  } else if (IS_NIL(value)) {
    printf("none");
  } else if (IS_NUMBER(value)) {
    printf("%.15g", AS_NUMBER(value));
  } else if (IS_OBJ(value)) {
    printObject(value);
  }
#else
  switch (value.type) {
    case VAL_BOOL:
      printf(AS_BOOL(value) ? "true" : "false");
      break;
    case VAL_NIL: printf("nil"); break;
    case VAL_NUMBER: printf("%.15g", AS_NUMBER(value)); break;
//> Strings call-print-object
    case VAL_OBJ: printObject(value); break;
//< Strings call-print-object
  }
//< Types of Values print-value
//> Optimization end-print-value
#endif
//< Optimization end-print-value
}
//< print-value
//> Types of Values values-equal
bool valuesEqual(Value a, Value b) {
#ifdef NAN_BOXING
  if (IS_OBJ(a) && IS_OBJ(b)) {
    if (AS_OBJ(a)->type != AS_OBJ(b)->type) return false;

    if (IS_LIST(a)) {
      ObjList* listA = AS_LIST(a);
      ObjList* listB = AS_LIST(b);

      if (listA->items.count != listB->items.count) {
        return false;
      }

      for (int i = 0; i < listA->items.count; i++) {
        if (!valuesEqual(listA->items.values[i], listB->items.values[i])) {
          return false;
        }
      }

      return true;
    }
  }


  if (IS_NUMBER(a) && IS_NUMBER(b)) {
    return AS_NUMBER(a) == AS_NUMBER(b);
  }


  return a == b;
#else
//< Optimization values-equal
  if (a.type != b.type) return false;
  switch (a.type) {
    case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
    case VAL_NIL:    return true;
    case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);

    case VAL_OBJ:    return AS_OBJ(a) == AS_OBJ(b);
//< Hash Tables equal
    default:         return false; // Unreachable.
  }
//> Optimization end-values-equal
#endif
//< Optimization end-values-equal
}
//< Types of Values values-equal
