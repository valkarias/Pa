#ifndef pcrap_tools_h
#define pcrap_tools_h

#include <sys/unistd.h>
#include <string.h>

#include "common.h"
#include "memory.h"

#define _basename(c1, c2, c3, c4, c5) _splitpath(c1, c2, c3, c4, c5)

#ifdef _WIN32
#define SEP_C "\\"
#define SEP '\\'
#else
#define SEP_C "/"
#define SEP '/'
#endif

#define _MAX 260

char* readFile(char* path);
bool checkPath(char* filename);

char* basename(char* path);
char* resolveLibrary(char* name);
#endif