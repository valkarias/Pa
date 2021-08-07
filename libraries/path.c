#include "pcrap_path.h"

static Value basenameLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'basename()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument must be a string from 'basename()'.");
        return NOTCLEAR;
    }

    char* path = AS_CSTRING(args[0]);

    if (path[0] == '\0') {
        return FAILED;
    }

    char* base = basename(path);
    return OBJ_VAL(takeString(base, strlen(base)));
}

static Value extLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'extension()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument must be a string from 'extension()'.");
        return NOTCLEAR;
    }

    char* path = AS_CSTRING(args[0]);

    const char* d = strrchr(path, '.');
    if (!d || d == path) return FAILED;
    return OBJ_VAL(copyString(d + 1, strlen(d + 1)));
}

static Value dirLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'dirname()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument must be a string from 'dirname()'.");
        return NOTCLEAR;
    }

    char* path = AS_CSTRING(args[0]);

    if (path[0] == '\0') {
        return FAILED;
    }

    char* dir = ALLOCATE(char, strlen(path) + 1);
    memcpy(dir, path, strlen(path));
    dir[strlen(path)] = '\0';

    _dirname(dir);
    return OBJ_VAL(takeString(dir, strlen(dir)));
}

static Value isDirLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'isDir()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument must be a string from 'isDir()'.");
        return NOTCLEAR;
    }

    char *path = AS_CSTRING(args[0]);

    struct stat path_stat;
    stat(path, &path_stat);

    if (S_ISDIR(path_stat.st_mode)) {
        return TRUE_VAL;
    }

    return FALSE_VAL;
}

static Value isFileLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'isFile()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument must be a string from 'isFile()'.");
        return NOTCLEAR;
    }

    char *path = AS_CSTRING(args[0]);

    struct stat path_stat;
    stat(path, &path_stat);

    if (S_ISREG(path_stat.st_mode)) {
        return TRUE_VAL;
    }

    return FALSE_VAL;
}

//
ObjLibrary* createPathLibrary() {
    ObjString* name = copyString("Path", 4);
    push(OBJ_VAL(name));
    ObjLibrary* library = newLibrary(name);
    push(OBJ_VAL(library));

    defineNative("basename", basenameLib, &library->values);
    defineNative("extension", extLib, &library->values);
    defineNative("dirname", dirLib, &library->values);
    defineNative("isDir", isDirLib, &library->values);
    defineNative("isFile", isFileLib, &library->values);

#ifdef _WIN32
    defineProperty("separator", OBJ_VAL(copyString("\\", 1)), &library->values);
#else
    defineProperty("separator", OBJ_VAL(copyString("/", 1)), &library->values);
#endif
    
    pop();
    pop();

    return library;
    
}