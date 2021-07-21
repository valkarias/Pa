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

    time_t t;
    srand((unsigned)time(&t));

    int res = rand() % (max + 1 - min) + min;
    return NUMBER_VAL(res);
}

static Value choiceLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument1 but got %d from 'choice()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_LIST(args[0])) {
        runtimeError("Argument must be a list from 'choice()'.");
        return NOTCLEAR;
    }    

    ObjList* list = AS_LIST(args[0]);
    argCount = list->items.count;
    args = list->items.values;

    time_t t;
    srand((unsigned)time(&t));

    int r = rand();
    int res = r % argCount;

    return args[res];
}

//
ObjLibrary* createRandomLibrary() {
    ObjString* name = copyString("Random", 6);
    push(OBJ_VAL(name));
    ObjLibrary* library = newLibrary(name);
    push(OBJ_VAL(library));

    defineNative("range", rangeLib, &library->values);
    defineNative("choice", choiceLib, &library->values);
    
    pop();
    pop();

    return library;
    
}