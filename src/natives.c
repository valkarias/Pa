#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "value.h"
#include "natives.h"
#include "vm.h"
#include "memory.h"

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
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'assert()'.", argCount);
        return NOTCLEAR;
    }

    if (isFalsey(args[0])) {
        ERROR("Assertion Failed");
        return NOTCLEAR;
    }

    return CLEAR;
}

static Value assertShowNative(int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d from 'assertShow()'.", argCount);
        return NOTCLEAR;
    }
    if (!IS_STRING(args[1])) {
        runtimeError("Argument must be a string from 'assertShow()'.");
        return NOTCLEAR;
    }

    if (isFalsey(args[0])) {
        ERROR(AS_CSTRING(args[1]));
        return NOTCLEAR;
    }

    return CLEAR;
}

///////////////////

void defineAllNatives() {
    char* nativeStrings[] = {
        "input",
        "type",

        "assert",
        "assertShow"
    };

    NativeFn nativeFunctions[] = {
        inputNative,
        typeNative,
        
        assertNative,
        assertShowNative,
    };

    for (uint8_t i = 0; i < sizeof(nativeStrings) / sizeof(nativeStrings[0]); i++) {
        defineNative(nativeStrings[i], nativeFunctions[i], &vm.globals);
    }
}