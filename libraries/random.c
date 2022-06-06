#include "random.h"

// hmmm....
static unsigned int g_seed;

static inline void fastsrand(int seed) {
  g_seed = seed;
}

static inline int fastrand() { 
  g_seed = (214013 * g_seed + 2531011); 
  return (g_seed >> 16) & 0x7FFF; 
}
//

static Value fastRangeLib(int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d from 'fastRange()'.", argCount);
        return NOTCLEAR;
    }
    if (!IS_NUMBER(args[0])) {
        runtimeError("First argument must be a number from 'fastRange()'.");
        return NOTCLEAR;
    }
    if (!IS_NUMBER(args[1])) {
        runtimeError("Second argument must be a number from 'fastRange()'.");
        return NOTCLEAR;
    }

    int min = AS_NUMBER(args[0]);
    int max = AS_NUMBER(args[1]);

    fastsrand((int)time(NULL));
    return NUMBER_VAL(fastrand() % (max + 1 - min) + min);
}

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

    srand((unsigned)time(NULL));
    return NUMBER_VAL(rand() % (max + 1 - min) + min);
}

static Value choiceLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'choice()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_LIST(args[0])) {
        runtimeError("Argument must be a list from 'choice()'.");
        return NOTCLEAR;
    }    

    ObjList* list = AS_LIST(args[0]);

    if (list->items.count == 0) {
        runtimeError("List should not be empty from 'choice()'.");
        return NOTCLEAR;
    }

    argCount = list->items.count;
    args = list->items.values;

    int r = rand();
    int res = r % argCount;

    return args[res];
}

static Value fillLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'fill()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number from 'fill()'.");
        return NOTCLEAR;
    }

    double count = AS_NUMBER(args[0]);
    ObjList* result = newList();
    push(OBJ_VAL(result));

    srand((unsigned)time(NULL));

    for (int i = 0; i < count; i++) {
        int r = rand() % RAND_MAX;
        appendToList(result, NUMBER_VAL(r));
    }
    pop();

    return OBJ_VAL(result);
}

//
ObjLibrary* createRandomLibrary() {
    //TODO: add xoroshiro algorithm?
    //TODO: benchmark range & fastRange.
    ObjString* name = copyString("Random", 6);
    push(OBJ_VAL(name));
    ObjLibrary* library = newLibrary(name);
    push(OBJ_VAL(library));

    defineNative("range", rangeLib, &library->values);
    defineNative("fastRange", fastRangeLib, &library->values);
    defineNative("choice", choiceLib, &library->values);
    defineNative("fill", fillLib, &library->values);
    
    defineProperty("RANDOM_MAX", NUMBER_VAL(RAND_MAX), &library->values);

    pop();
    pop();

    return library;
    
}