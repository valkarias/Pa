#ifndef Pa_path_h
#define Pa_path_h

#include "../src/object.h"
#include "../src/value.h"
#include "../src/vm.h"
#include "../src/tools.h"

#include "library.h"

#ifdef _WIN32
    #include "win.h"
#else
    #include <dirent.h>
#endif
#undef TokenType

ObjLibrary* createPathLibrary();

#endif
