#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "value.h"
#include "natives.h"
#include "vm.h"
#include "memory.h"


static Value printNative(int argCount, Value* args) {
    if (argCount == 0) {
        printf("\n");
        return CLEAR;
    }

    for (int i = 0; i < argCount; i++) {
        printValue(args[i]);
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
            runtimeError("input() only takes a string argument");
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

///////////////////

void defineAllNatives() {
    char* nativeStrings[] = {
        "printn",
        "input",
    };

    NativeFn nativeFunctions[] = {
        printNative,
        inputNative,
    };

    for (uint8_t i = 0; i < sizeof(nativeStrings) / sizeof(nativeStrings[0]); i++) {
        defineNative(nativeStrings[i], nativeFunctions[i], &vm.globals);
    }
}