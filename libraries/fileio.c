#include "fileio.h"

static Value openLib(int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d from 'open()'.", argCount);
        return NOTCLEAR;
    }
    if (!IS_STRING(args[0])) {
        runtimeError("First argument must be a string from 'open()'.");
        return NOTCLEAR;
    }
    if (!IS_STRING(args[1])) {
        runtimeError("Second argument must be a string from 'open()'.");
        return NOTCLEAR;
    }

    ObjString* fileName = AS_STRING(args[0]);
    ObjString* openType = AS_STRING(args[1]);

    ObjFile* file = newFile();
    file->file = fopen(
        fileName->chars, openType->chars
    );
    file->path = fileName->chars;
    file->openType = openType->chars;

    if (!file->file) {
        runtimeError("Unable to open file '%s'.", file->path);
        info("Double check the file path!");
        info("Also double check the file open mode");
        info("Function signature: open(\"%s\", \"%s\")", file->path, file->openType);
        return NOTCLEAR;
    }

    return OBJ_VAL(file);
}

static Value writeLib(int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d from 'write()'.", argCount);
        return NOTCLEAR;
    }
    if (!IS_FILE(args[0])) {
        runtimeError("First argument must be a file object from 'write()'.");
        return NOTCLEAR;
    }
    if (!IS_STRING(args[1])) {
        runtimeError("Second argument must be a string from 'write()'.");
        return NOTCLEAR;
    }

    ObjFile* file = AS_FILE(args[0]);
    ObjString* string = AS_STRING(args[1]);

    if (strcmp(file->openType, "r") == false) {
        runtimeError("File is not writable.");
        info("The file is opened in the '%s' mode!", file->openType);
        return NOTCLEAR;
    }

    int wrote = fprintf(file->file, "%s", string->chars);
    fflush(file->file);

    return NUMBER_VAL(wrote);
}

static Value readLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'read()'.", argCount);
        return NOTCLEAR;
    }
    if (!IS_FILE(args[0])) {
        runtimeError("Argument must be a file object from 'read()'.");
        return NOTCLEAR;
    }

    ObjFile* file = AS_FILE(args[0]);
    size_t curPos = ftell(file->file);
    fseek(file->file, 0L, SEEK_END);
    size_t size = ftell(file->file);
    fseek(file->file, curPos, SEEK_SET);

    char* buffer = ALLOCATE(char, size + 1);
    if (!buffer) {
        runtimeError("Could not read the file '%s' due to memory issues.", file->path);
        return NOTCLEAR;
    }

    size_t read = fread(buffer, sizeof(char), size, file->file);
    if (read < size && !feof(file->file)) {
        FREE_ARRAY(char, buffer, size+1);
        runtimeError("Could not read the file '%s'.", file->path);
        return NOTCLEAR;
    }

    if (read != size) {
        //shrink
        buffer = GROW_ARRAY(char, buffer, size + 1, read + 1);
    }

    buffer[read] = '\0';
    return OBJ_VAL(takeString(buffer, read));
}

static Value isEOFLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'isEOF()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_FILE(args[0])) {
        runtimeError("Argument must be a file object from 'isEOF()'.");
        return NOTCLEAR;
    }

    ObjFile* file = AS_FILE(args[0]);
    int value = feof(file->file);
    return BOOL_VAL(value != 0); 
}

static Value seekLib(int argCount, Value *args) {
    if (argCount != 3) {
        runtimeError("Expected 3 arguments but got %d from 'seek()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_FILE(args[0])) {
        runtimeError("First argument must be a file object from 'seek()'.");
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        runtimeError("First && Second arguments must be file objects from 'seek()'.");
        return NOTCLEAR;
    }

    ObjFile* file = AS_FILE(args[0]);
    int offset = AS_NUMBER(args[1]);
    int type = AS_NUMBER(args[2]);

    fseek(file->file, offset, type);
    return CLEAR;
}

static Value closeLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'close()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_FILE(args[0])) {
        runtimeError("Argument must be a file object from 'close()'.");
        return NOTCLEAR;
    }


    ObjFile* file = AS_FILE(args[0]);
    fclose(file->file);
    return CLEAR;
}

ObjLibrary* createFileioLibrary() {
    ObjString* name = copyString("File", 4);
    push(OBJ_VAL(name));
    ObjLibrary* library = newLibrary(name);
    push(OBJ_VAL(library));

    defineNative("open", openLib, &library->values);
    defineNative("close", closeLib, &library->values);
    defineNative("write", writeLib, &library->values);
    defineNative("read", readLib, &library->values);
    defineNative("seek", seekLib, &library->values);
    defineNative("isEOF", isEOFLib, &library->values);
    
    defineProperty("SEEK_SET", NUMBER_VAL(SEEK_SET), &library->values);
    defineProperty("SEEK_CUR", NUMBER_VAL(SEEK_CUR), &library->values);
    defineProperty("SEEK_END", NUMBER_VAL(SEEK_END), &library->values);

    pop();
    pop();

    return library;
}