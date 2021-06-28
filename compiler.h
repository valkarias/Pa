//> Scanning on Demand compiler-h
#ifndef pcrap_compiler_h
#define pcrap_compiler_h

//> Strings compiler-include-object
#include "object.h"
//< Strings compiler-include-object
//> Compiling Expressions compile-h
#include "vm.h"

#define MAX_NUMBER_VALUE 1000000
#define skip - 1

ObjFunction* compile(const char* source, ObjLibrary* library);
//< Calls and Functions compile-h
//> Garbage Collection mark-compiler-roots-h
void markCompilerRoots();
//< Garbage Collection mark-compiler-roots-h

#endif
