//> Scanning on Demand compiler-h
#ifndef pcrap_compiler_h
#define pcrap_compiler_h


#include "object.h"
#include <ctype.h>

#define MAX_NUMBER_VALUE 1000000
#define skip -1
#define NONE {NULL,     NULL,   PREC_NONE}

ObjFunction* compile(const char* source, ObjLibrary* library);
void markCompilerRoots();

#endif
