#ifndef pcrap_time_h
#define pcrap_time_h

#include <time.h>

#include "../src/object.h"
#include "../src/value.h"
#include "../src/vm.h"

#include "../src/memory.h"

#include "library.h"

ObjLibrary* createTimeLibrary();

#endif