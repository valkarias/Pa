#ifndef Pa_os_h
#define Pa_os_h

//combining mix functionalities into 1 module.
#include <sys/stat.h>

#ifdef _WIN32
    #include <io.h>
    #include <direct.h>
#elif __APPLE__
    #include <sys/uio.h>
    #include <unistd.h>
#else
    #include <sys/io.h>
    #include <unistd.h>
#endif

#include "../src/object.h"
#include "../src/tools.h"
#include "library.h"

//p = path, m = mode
#ifdef _WIN32
#define REMOVE(p) remove(p)
#define ACCESS(p, m) _access(p, m)
#define MKDIR(p, m) ((void)m, _mkdir(p))
#define rmdir(p) _rmdir(p)
#else
#define REMOVE(p) unlink(p)
#define ACCESS(p, m) access(p, m)
#define MKDIR(p, m) mkdir(p, m)
#define rmdir(p) rmdir(p)
#endif

ObjLibrary* createOsLibrary();

#endif
