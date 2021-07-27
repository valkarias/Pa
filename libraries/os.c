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
        runtimeError("Exit-status code maximum value is 255 from 'exit()'.");
        return NOTCLEAR;
    }

    exit(number);
    return CLEAR; // Thanks Dictu & Clang.
}

static Value timeLib(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'time()'.", argCount);
        return NOTCLEAR;
    }

    return NUMBER_VAL( (double) time(NULL) );
}


static Value clockLib(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'clock()'.", argCount);
        return NOTCLEAR;
    }

    return NUMBER_VAL( (double)clock() / CLOCKS_PER_SEC );
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

static Value mkdirLib(int argCount, Value* args) {
    if (argCount == 0 || argCount > 2) {
        runtimeError("Expected 1 or 2 arguments but got %d from 'mkdir()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_STRING(args[0])) {
        runtimeError("First Argument must be a string from 'mkdir()'.");
        return NOTCLEAR;
    }

    char* p = AS_CSTRING(args[0]);
    int m = 0777;

    if (argCount == 2) {
        if (!IS_NUMBER(args[1])) {
            runtimeError("Second Argument must be a number from 'mkdir()'.");
            return NOTCLEAR;
        }

        m = AS_NUMBER(args[1]);
    }

    int status = MKDIR(p, m);
    if (status < 0) {
        return FAILED;
    }

    return CLEAR;
}

static Value rmdirLib(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'rmdir()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument must be a string from 'rmdir()'.");
        return NOTCLEAR;
    }

    char* p = AS_CSTRING(args[0]);
    
    int status = rmdir(p);
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

static Value setCwdLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'setCwd()'.", argCount);
        return NOTCLEAR;
    }

    char* p = AS_CSTRING(args[0]);

    int status = chdir(p);

    if (status < 0) {
        return FAILED;
    }

    return CLEAR;
}

static void mkdirs(const char* path) {
    char tmp[260]; //ugh
    char* p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    
    if(tmp[len - 1] == SEP) {
        tmp[len - 1] = 0;
    }

    for(p = tmp + 1; *p; p++) {
        if(*p == SEP) {
            *p = 0;
            MKDIR(tmp, S_IRWXU);
            *p = SEP;
        }
    }

    MKDIR(tmp, S_IRWXU);
}

static Value makeDirsLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'makeDirs()'.", argCount);
        return NOTCLEAR;
    }

    char* p = AS_CSTRING(args[0]);

    mkdirs(p);
    return CLEAR;
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
    return "Mac";
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
    defineNative("clock", clockLib, &library->values);
    defineNative("time", timeLib, &library->values);
    defineNative("remove", removeLib, &library->values);
    defineNative("getCwd", getCwdLib, &library->values);
    defineNative("setCwd", setCwdLib, &library->values);
    defineNative("getHome", getHomeLib, &library->values);
    defineNative("mkdir", mkdirLib, &library->values);
    defineNative("rmdir", rmdirLib, &library->values);
    defineNative("makeDirs", makeDirsLib, &library->values);

    defineProperty("F_OK", NUMBER_VAL(F_OK), &library->values);
    defineProperty("X_OK", NUMBER_VAL(X_OK), &library->values);
    defineProperty("W_OK", NUMBER_VAL(W_OK), &library->values);
    defineProperty("R_OK", NUMBER_VAL(R_OK), &library->values);

#ifdef _WIN32
    defineProperty("separator", OBJ_VAL(copyString("\\", 1)), &library->values);
#else
    defineProperty("separator", OBJ_VAL(copyString("/", 1)), &library->values);
#endif

    char* pChar = getPlatform();
    ObjString* platform = copyString(pChar, strlen(pChar));
    push(OBJ_VAL(platform));
    defineProperty("name", OBJ_VAL(platform), &library->values);
    pop();

    pop();
    pop();

    return library;
}