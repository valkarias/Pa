#include "random.h"

static Value rangeLib(int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d from 'range()'.", argCount);
        return NOTCLEAR;
    }
    if (!IS_NUMBER(args[0])) {
        runtimeError("First argument must be a number from 'range()'.");
        return NOTCLEAR;
    }
    if (!IS_NUMBER(args[1])) {
        runtimeError("Second argument must be a number from 'range()'.");
        return NOTCLEAR;
    }

    int min = AS_NUMBER(args[0]);
    int max = AS_NUMBER(args[1]);

    time_t time;
    srand((unsigned)time);

    int res = rand() % (max + 1 - min) + min;
    return NUMBER_VAL(res);
}

//
ObjLibrary* createRandomLibrary() {
    ObjString* name = copyString("Random", 6);
    push(OBJ_VAL(name));
    ObjLibrary* library = newLibrary(name);
    push(OBJ_VAL(library));

    defineNative("range", rangeLib, &library->values);
    
    pop();
    pop();

    return library;
    
}