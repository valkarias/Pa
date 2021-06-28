#ifndef pcrap_os_h
#define pcrap_os_h

#include <io.h>
#include "../object.h"
#include "library.h"

#ifdef _WIN32
#define REMOVE remove
#define ACCESS _access
#else
#define REMOVE unlink
#define ACCESS access
#endif

#define FAILED NUMBER_VAL(-1)

ObjLibrary* createOsLibrary();

#endif
