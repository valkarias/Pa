#ifndef Pa_library_h
#define Pa_library_h

#include "Pa_math.h"
#include "os.h"
#include "random.h"
#include "Pa_time.h"
#include "Pa_path.h"
#include "Pa_ascii.h"
#include "fileio.h"

typedef ObjLibrary *(*NativeLibrary)();

typedef struct {
    char *name;
    NativeLibrary library;
} NativeLibraries;

ObjLibrary* importLibrary(int index);
int getNativeModule(char* name, int length);

#define NOTCLEAR NIL_VAL
#define CLEAR NUMBER_VAL(0)
#define FAILED NUMBER_VAL(-1)

#endif
