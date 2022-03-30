#include "Pa_ascii.h"

char* toAscii(int n) {
    char* string = ALLOCATE(char, 2);
    string[2] = '\0';

    if (!string) {
        return 0;
    }

    string[0] = n;
    string[1] = 0;
    return string;
}

static Value codeLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'code()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_STRING(args[0])) {
        runtimeError("First argument must be a number from 'code()'.");
        return NOTCLEAR;
    }

    if (AS_STRING(args[0])->length > 1) {
        runtimeError("Expected 1 character but got a string of %d from 'code()'.", AS_STRING(args[0])->length);
        return NOTCLEAR;
    }

    ObjString* character = AS_STRING(args[0]);
    int code = character->chars[0];
    return NUMBER_VAL(code);
}

static Value asciiLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'ascii()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("First argument must be a number from 'ascii()'.");
        return NOTCLEAR;
    }

    int n = AS_NUMBER(args[0]);
    char* c = toAscii(n);
    return OBJ_VAL(takeString(c, strlen(c)));
}

//
ObjLibrary* createAsciiLibrary() {
    ObjString* name = copyString("Ascii", 5);
    push(OBJ_VAL(name));
    ObjLibrary* library = newLibrary(name);
    push(OBJ_VAL(library));

    defineNative("ascii", asciiLib, &library->values);
    defineNative("code", codeLib, &library->values);

    defineProperty("upper", OBJ_VAL(copyString("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26)), &library->values);
    defineProperty("lower", OBJ_VAL(copyString("abcdefghijklmnopqrstuvwxyz", 26)), &library->values);
    defineProperty("digits", OBJ_VAL(copyString("0123456789", 10)), &library->values);
    defineProperty("hex", OBJ_VAL(copyString("0123456789abcdefABCDEF", 22)), &library->values);
    defineProperty("octal", OBJ_VAL(copyString("01234567", 8)), &library->values);
    // local.
    defineProperty("punctuation", OBJ_VAL(copyString("!#$%%&'()*+,-./:;<=>?@[\\]^_`{|}~", 32)), &library->values);
    
    pop();
    pop();

    return library;
    
}