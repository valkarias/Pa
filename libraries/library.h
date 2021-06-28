#ifndef pcrap_library_h
#define pcrap_library_h

#include "math.h"
#include "os.h"

typedef ObjLibrary *(*NativeLibrary)();

typedef struct {
    char *name;
    NativeLibrary library;
} NativeLibraries;

ObjLibrary* importLibrary(int index);
int getNativeModule(char* name, int length);

#define NOTCLEAR NIL_VAL
#define CLEAR NUMBER_VAL(0)

#endif
