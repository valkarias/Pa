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

    if (list->items.count == 0) {
        runtimeError("Can not remove from an empty list from 'remove()'.", argCount);
        return NOTCLEAR;
    }

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
        Value last = list->items.values[list->items.count - 1];
        deleteFromList(list, list->items.count - 1);
        return last;
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

static ObjList* copyList(ObjList* list) {
    ObjList* copy = newList();
    const int length = list->items.count;
    push(OBJ_VAL(copy));

    for (int i = 0; i < length; i++) {
        Value val = list->items.values[i];

        if (list->items.count != 0) {
            if (IS_LIST(val)) {
                val = OBJ_VAL(copyList(AS_LIST(val)));
            }
        }

        push(val);
        appendToList(copy, val);
        pop();
    }

    pop();
    return copy;
}

static Value copyMethod(int argCount, Value* args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'copy()'.", argCount);
        return NOTCLEAR;
    }

    ObjList* list = AS_LIST(args[0]);
    return OBJ_VAL(copyList(list));
}

// O(n^2) time complexity...
static ObjList* flattenList(ObjList* list) {
    if (list->items.count <= 1) {
        return list;
    }

    ObjList* res = newList();
    push(OBJ_VAL(res));

    for (int i = 0; i < list->items.count; i++) {
        if (IS_LIST(list->items.values[i])) {
            ObjList* temp = flattenList(AS_LIST(list->items.values[i]));
            for (int j = 0; j < temp->items.count; j++) {
                appendToList(res, temp->items.values[j]);
            }
        } else {
            appendToList(res, list->items.values[i]);
        }
    }

    pop();
    return res;
}

static Value flattenMethod(int argCount, Value* args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'flatten()'.", argCount);
        return NOTCLEAR;
    }

    ObjList* list = AS_LIST(args[0]);
    return OBJ_VAL(flattenList(list));
}

static Value sliceMethod(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d from 'slice()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        runtimeError("First and second arguments must be numbers from 'slice()'.");
        return NOTCLEAR;
    }

    int start = AS_NUMBER(args[1]);
    int limit = AS_NUMBER(args[2]) + 1;

    ObjList* list = AS_LIST(args[0]);

    ObjList* res = newList();
    push(OBJ_VAL(res));

    if (start < 0) {
        start = 0;
    }
    if (list->items.count <= limit) {
        limit = list->items.count;
    }

    for (int i = start; i < limit; i++) {
        appendToList(res, list->items.values[i]);
    }

    pop(res);
    return OBJ_VAL(res);
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
        "copy",
        "flatten",
        "slice",
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
        reverseMethod,
        copyMethod,
        flattenMethod,
        sliceMethod,
    };

    for (uint8_t i = 0; i < sizeof(listMethodStrings) / sizeof(listMethodStrings[0]); i++) {
        defineNative(listMethodStrings[i], listMethods[i], &vm.listNativeMethods);
    }

    interpret(LIST_EXTRA, "List");

    Value val;
    tableGet(&vm.libraries, copyString("List", 4), &val);

    ObjLibrary* library = AS_LIBRARY(val);
    push(val);
    tableAddAll(&library->values, &vm.listNativeMethods);
    pop();
}