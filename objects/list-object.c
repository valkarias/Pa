#include "objects.h"

static Value appendMethod(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'append()'.", argCount);
        return NOTCLEAR;
    }

    ObjList* list = AS_LIST(args[0]);
    Value item = args[1];

    appendToList(list, item);

    return CLEAR;
}

static Value containMethod(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'contains()'.", argCount);
        return NOTCLEAR;
    }

    ObjList* list = AS_LIST(args[0]);
    Value item = args[1];

    for (int i = 0; i < list->items.count; i++) {
        if (valuesEqual(list->items.values[i], item)) {
            return TRUE_VAL;
        }
    }

    return FALSE_VAL;
}

static Value removeMethod(int argCount, Value *args) {
    if (argCount > 1) {
        runtimeError("Expected 1 or 0 arguments but got %d from 'remove()'.", argCount);
        return NOTCLEAR;
    }

    ObjList* list = AS_LIST(args[0]);

    if (argCount == 1) {
        if (!IS_NUMBER(args[1])) {
            runtimeError("Index must be a number from 'remove()'.");
            return NOTCLEAR;
        }

        int index = AS_NUMBER(args[1]);

        if (!isValidListIndex(list ,index)) {
            runtimeError("Index out of bounds.");
            return NOTCLEAR;
        }

        deleteFromList(list, index);
        return CLEAR;
    } else {
        deleteFromList(list, list->items.count - 1);
        return CLEAR;
    }

    return CLEAR; // Clang
}

static Value allMethod(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'length()'.", argCount);
        return NOTCLEAR;
    }

    ObjList* list = AS_LIST(args[0]);

    for (int i = 0; i < list->items.count; i++) {
        if (isFalsey(list->items.values[i])) {
            return FALSE_VAL;
        }
    }    

    return TRUE_VAL;
}

static Value anyMethod(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'any()'.", argCount);
        return NOTCLEAR;
    }

    ObjList* list = AS_LIST(args[0]);

    for (int i = 0; i < list->items.count; i++) {
        if (!isFalsey(list->items.values[i])) {
            return TRUE_VAL;
        }
    }    

    return FALSE_VAL;
}

static Value lengthMethod(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'length()'.", argCount);
        return NOTCLEAR;
    }

    ObjList* list = AS_LIST(args[0]);

    return NUMBER_VAL(list->items.count);
}

static Value indexMethod(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'index()'.", argCount);
        return NOTCLEAR;
    }

    ObjList* list = AS_LIST(args[0]);
    Value item = args[1];
    
    for (int i = 0; i < list->items.count; i++) {
        if (valuesEqual(list->items.values[i], item)) {
            return NUMBER_VAL(i);
        }
    }

    return FALSE_VAL;
}

static Value clearMethod(int argCount, Value* args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'clean()'.", argCount);
        return NOTCLEAR;
    }

    clearList(AS_LIST(args[0]));
    return CLEAR;
}

static Value reverseMethod(int argCount, Value* args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'reverse()'.", argCount);
        return NOTCLEAR;
    }

    ObjList* list = AS_LIST(args[0]);
    int len = list->items.count;
    int j;

    for(int i = 0, j = len - 1; i < j; i++, j--) {
        Value temp = list->items.values[i];

        list->items.values[i] = list->items.values[j];
        list->items.values[j] = temp;
    }

    return CLEAR;
}

//
void initListMethods() {
    char* listMethodStrings[] = {
        "append",
        "length",
        "remove",
        "contains",
        "index",
        "clear",

        "all",
        "any",
        
        "reverse",
    };

    NativeFn listMethods[] = {
        appendMethod,
        lengthMethod,
        removeMethod,
        containMethod,
        indexMethod,
        clearMethod,

        allMethod,
        anyMethod,

        reverseMethod
    };

    for (uint8_t i = 0; i < sizeof(listMethodStrings) / sizeof(listMethodStrings[0]); i++) {
        defineNative(listMethodStrings[i], listMethods[i], &vm.listNativeMethods);
    }
}