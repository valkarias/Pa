#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "value.h"
#include "natives.h"
#include "vm.h"
#include "memory.h"

static Value printNative(int argCount, Value *args) {
    if (argCount == 0 || argCount > 2) {
        runtimeError("Expected 1 or 2 arguments but got %d from 'print()'.", argCount);
        return NOTCLEAR;
    }

    printValue(args[0]);
    if (argCount == 2) {
        if (!IS_STRING(args[1])) {
            runtimeError("Second Argument must be a string from 'print()'.");
            return NOTCLEAR;
        }
        printValue(args[1]);
    } else {
        printf("\n");
    }

    return CLEAR;
}


static Value inputNative(int argCount, Value *args) {
    if (argCount > 1) {
        runtimeError("Expected 1 or 0 arguments but got %d from 'input()'.", argCount);
        return NOTCLEAR;
    }

    if (argCount != 0) {
        Value input = args[0];
        if (!IS_STRING(input)) {
            runtimeError("Argument must be a string from 'input()'.");
            return NOTCLEAR;
        }

        printf("%s", AS_CSTRING(input));
    }

    uint64_t currentSize = 128;
    char *line = ALLOCATE(char, currentSize);

    if (line == NULL) {
        runtimeError("A Memory error occured on input()!?");
        return NOTCLEAR;
    }

    int c = EOF;
    uint64_t length = 0;
    while ((c = getchar()) != '\n' && c != EOF) {
        line[length++] = (char) c;

        if (length + 1 == currentSize) {
            int oldSize = currentSize;
            currentSize = GROW_CAPACITY(currentSize);
            line = GROW_ARRAY(char, line, oldSize, currentSize);

            if (line == NULL) {
                printf("Unable to allocate more memory\n");
                exit(71);
            }
        }
    }

    if (length != currentSize) {
        line = GROW_ARRAY(char, line, currentSize, length + 1);
    }

    line[length] = '\0';

    return OBJ_VAL(takeString(line, length));
}

static Value typeNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'type()'.", argCount);
        return NOTCLEAR;
    }

    char* c = typeValue(args[0]);
    return OBJ_VAL(takeString(c, strlen(c)));
}

static Value assertNative(int argCount, Value *args) {
    if (argCount != 1 && argCount != 2) {
        runtimeError("Expected 1 or 2 arguments but got %d from 'assert()'.", argCount);
        return NOTCLEAR;
    }

    char* message = "No Source.";
    if (!IS_BOOL(args[0])) {
        runtimeError("First argument must be a boolean from 'assert()'.");
        return NOTCLEAR;
    }

    if (argCount == 2) {
        if (!IS_STRING(args[1])) {
            runtimeError("Second argument must be a string from 'assert()'.");
            return NOTCLEAR;
        }

        message = AS_CSTRING(args[1]);
    }

    if (isFalsey(args[0])) {
        ERROR(message);
        return NOTCLEAR;
    }

    return CLEAR;
}

static Value toStringNative(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'toString()'.", argCount);
        return NOTCLEAR;
    }

    if (IS_STRING(args[0])) {
        return args[0];
    }

    char* c = stringValue(args[0]);
    return OBJ_VAL(takeString(c, strlen(c)));
}


///////////////////

void defineAllNatives() {
    char* nativeStrings[] = {
        "print",
        "input",
        "type",
        "toString",
        "assert",
    };

    NativeFn nativeFunctions[] = {
        printNative,
        inputNative,
        typeNative,
        toStringNative,
        assertNative,
    };

    for (uint8_t i = 0; i < sizeof(nativeStrings) / sizeof(nativeStrings[0]); i++) {
        defineNative(nativeStrings[i], nativeFunctions[i], &vm.globals);
    }
}