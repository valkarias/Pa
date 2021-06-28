#include "os.h"

static Value exitLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'exit()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number from 'exit()'.");
        return NOTCLEAR;
    }

    int number = AS_NUMBER(args[0]);

    if (number > 255 || number < 0) {
        runtimeError("max exit-status is 255 & min is 0.");
        return NOTCLEAR;
    }

    exit(number);
    return CLEAR; // Thanks Dictu & tcc.
}

static Value removeLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'remove()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument must be a string from 'remove()'.");
        return NOTCLEAR;
    }

    char* path = AS_CSTRING(args[0]);

    int status = REMOVE(path);
    if (status < 0) {
        return FAILED;
    }
    
    return CLEAR;
}

static Value getCwdLib(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'getCwd()'.", argCount);
        return NOTCLEAR;
    }

    char cwdPath[100];

    if (getcwd(cwdPath, sizeof(cwdPath)) != NULL) {
        return OBJ_VAL(copyString(cwdPath, strlen(cwdPath)));
    }

    return FAILED;
}

static Value accessLib(int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d from 'access()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_STRING(args[0])) {
        runtimeError("First argument must be a string from 'access()'.");
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError("Second argument must be a number from 'access()'");
        return NOTCLEAR;
    }

    char* path = AS_CSTRING(args[0]);
    int mode = AS_NUMBER(args[1]);

    int status = ACCESS(path, mode);
    if (status < 0) {
        return FAILED;
    }

    return CLEAR;
}

static Value getHomeLib(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'getHome()'.", argCount);
        return NOTCLEAR;
    }

    char home[260];

#ifdef _WIN32
    snprintf(home, 260, "%s%s", getenv("HOMEDRIVE"), getenv("HOMEPATH"));
#else
    snprintf(home, 260, "%s", getenv("HOME"));
#endif

    return OBJ_VAL(copyString(home, strlen(home)));
}

//

static char* getPlatform() {
#ifdef _WIN32
    return "Windows 32";
#elif _WIN64
    return "Windows 64";
#elif __linux__
    return "Linux";
#elif __unix || __unix__
    return "Unix";
#elif __APPLE__
    return "Mac"
#else
    return "unknown";
#endif
}

ObjLibrary* createOsLibrary() {
    ObjString* name = copyString("Os", 2);
    push(OBJ_VAL(name));
    ObjLibrary* library = newLibrary(name);
    push(OBJ_VAL(library));

#ifndef __APPLE__
    defineNative("access", accessLib, &library->values);
#endif
    defineNative("exit", exitLib, &library->values);
    defineNative("remove", removeLib, &library->values);
    defineNative("getCwd", getCwdLib, &library->values);
    defineNative("getHome", getHomeLib, &library->values);

    defineProperty("F_OK", NUMBER_VAL(F_OK), &library->values);
    defineProperty("X_OK", NUMBER_VAL(X_OK), &library->values);
    defineProperty("W_OK", NUMBER_VAL(W_OK), &library->values);
    defineProperty("R_OK", NUMBER_VAL(R_OK), &library->values);

    char* pChar = getPlatform();
    ObjString* platform = copyString(pChar, strlen(pChar));
    push(OBJ_VAL(platform));
    defineProperty("name", OBJ_VAL(platform), &library->values);
    pop();

    pop();
    pop();

    return library;
}