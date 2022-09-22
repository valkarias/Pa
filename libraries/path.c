#include "Pa_path.h"

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
        return NUMBER_VAL(-1);
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
    if (!d || d == path) return NUMBER_VAL(-1);
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
        return NUMBER_VAL(-1);
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

static Value realLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'real()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument must be a string from 'real()'.");
        return NOTCLEAR;
    }

    char* path = AS_CSTRING(args[0]);
    char* realpath = real(path);
    return OBJ_VAL(copyString(realpath, strlen(realpath)));
}

#ifdef _WIN32
    static bool winListDir(ObjList* list, const char* path) {
        WIN32_FIND_DATAA fdFile;
        HANDLE dir = NULL;

        // '\\*.*' 5 + 1 (\0)
        int length = strlen(path) + 6;
        char* searchPath = ALLOCATE(char, length);
        if (!searchPath) {
            runtimeError("Memory error on listDir()!?");
            return false;
        }
        strcpy(searchPath, path);
        strcat(searchPath, "\\*.*");

        if ( (dir = FindFirstFile(searchPath, &fdFile)) == INVALID_HANDLE_VALUE) {
            free(searchPath);
            FindClose(dir);
            return false;
        }

        do {
            // Ignore first entries ("." & "..")
            if (strcmp(fdFile.cFileName, ".") == 0 || strcmp(fdFile.cFileName, "..") == 0) {
                continue;
            }

            Value fileValue = OBJ_VAL(copyString(fdFile.cFileName, strlen(fdFile.cFileName)));
            push(fileValue);
            writeValueArray(&list->items, fileValue);
            pop();
        } while (FindNextFile(dir, &fdFile) != 0);

        FindClose(dir);
    }
#endif

static Value listDirLib(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'listDir()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_STRING(args[0])) {
        runtimeError("Argument must be a string from 'listDir()'.");
        return NOTCLEAR;
    }

    char* path = AS_CSTRING(args[0]);
    ObjList* contents = newList();
    push(OBJ_VAL(contents));

#ifdef _WIN32
    // winListDir modifies contents.
    if (!winListDir(contents, path)) {
        return NUMBER_VAL(-1);
    }
#else
    DIR* dir = opendir(path);
    struct dirent* dir_content;
    if (dir) {
        while ( (dir_content = readdir(dir)) != NULL) {
            char* node_name = dir_content->d_name;
            if (strcmp(node_name, ".") == 0 || strcmp(node_name, "..") == 0) {
                continue;
            }

            Value value = OBJ_VAL(copyString(node_name, strlen(node_name)));
            push(value);
            writeValueArray(&contents->items, value);
            pop();
        }
        closedir(dir);
    } else {
        closedir(dir);
        return NUMBER_VAL(-1);
    }
#endif

    pop();
    return OBJ_VAL(contents); 
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
    defineNative("listDir", listDirLib, &library->values);
    defineNative("isFile", isFileLib, &library->values);
    defineNative("real", realLib, &library->values);

#ifdef _WIN32
    defineProperty("separator", OBJ_VAL(copyString("\\", 1)), &library->values);
#else
    defineProperty("separator", OBJ_VAL(copyString("/", 1)), &library->values);
#endif
    
    pop();
    pop();

    return library;
    
}