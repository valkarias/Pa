#include "objects.h"
#include "../memory.h"

static Value lengthMethod(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'length()'.", argCount);
        return NOTCLEAR;
    }

    ObjString* string = AS_STRING(args[0]);
    return NUMBER_VAL(string->length);
}

static Value numberMethod(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'number()'.", argCount);
        return NOTCLEAR;
    } 

    char* numString = AS_CSTRING(args[0]);
    char* err;

    double num = strtod(numString, &err);

    if (err == numString) {
        runtimeError("A Conversion error occured on number()?!");
        return NOTCLEAR;
    }

    return NUMBER_VAL(num);
}

static Value splitMethod(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'split()'.", argCount);
        return NOTCLEAR;
    } 

    if (!IS_STRING(args[1])) {
        runtimeError("Argument must be a string from 'split()'.");
        return NOTCLEAR;
    }

    ObjString* st = AS_STRING(args[0]);
    char* dl = AS_CSTRING(args[1]);

    char* alloc = ALLOCATE(char, st->length + 1);
    char* ref = alloc;

    memmove(alloc, st->chars, st->length);
    alloc[st->length] = '\0';

    char* final;

    ObjList* l = newList();
    push(OBJ_VAL(l));

    if (strlen(dl) != 0) {
        do {
            final = strstr(alloc, dl);

            if (final) *final = '\0';

            Value objStr = OBJ_VAL(copyString(alloc, strlen(alloc)));

            push(objStr);
            appendToList(l, objStr);
            pop();

            alloc = final + strlen(dl);
        } while (final != NULL);
    }

    pop();
    FREE_ARRAY(char, ref, st->length + 1);
    return OBJ_VAL(l);
}


//
void initStringMethods() {
    char* stringMethodStrings[] = {
        "number",
        "length",
        "split"
    };

    NativeFn stringMethods[] = {
        numberMethod,
        lengthMethod,
        splitMethod,
    };

    for (uint8_t i = 0; i < sizeof(stringMethodStrings) / sizeof(stringMethodStrings[0]); i++) {
        defineNative(stringMethodStrings[i], stringMethods[i], &vm.stringNativeMethods);
    }
}