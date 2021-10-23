#include <string.h>

#include "objects.h"
#include "../src/memory.h"

static Value stringMethod(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'string()'.", argCount);
        return NOTCLEAR;
    }

    double num = AS_NUMBER(args[0]);
    int numLength = snprintf(NULL , 0, "%.15g", num) + 1;

    char* numString = ALLOCATE(char, numLength);

    if (numString == NULL) {
        runtimeError("A Memory error occured on string()!?");
        return NOTCLEAR;
    }

    snprintf(numString, numLength, "%.15g", num);
    return OBJ_VAL(takeString(numString, numLength - 1)); 
}

static Value boolMethod(int argCount, Value* args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'bool()'.", argCount);
        return NOTCLEAR;
    }

    double num = AS_NUMBER(args[0]);
    if (num) {
        return TRUE_VAL;
    }

    return FALSE_VAL;
}

//
void initNumberMethods() {
    char* numberMethodStrings[] = {
        "string",
        "bool",
    };

    NativeFn numberMethods[] = {
        stringMethod,
        boolMethod
    };

    for (uint8_t i = 0; i < sizeof(numberMethodStrings) / sizeof(numberMethodStrings[0]); i++) {
        defineNative(numberMethodStrings[i], numberMethods[i], &vm.numberNativeMethods);
    }

    interpret(NUMBER_EXTRA, "Number");

    Value val;
    tableGet(&vm.libraries, copyString("Number", 6), &val);

    ObjLibrary* library = AS_LIBRARY(val);
    push(val);
    tableAddAll(&library->values, &vm.numberNativeMethods);
    pop();
}