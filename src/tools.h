#ifndef pcrap_tools_h
#define pcrap_tools_h

#include <sys/unistd.h>
#include <string.h>

#include "common.h"
#include "memory.h"

#define _basename(c1) _splitpath(c1, NULL, NULL, c1, NULL)

#ifdef _WIN32
#define SEP_C "\\"
#define SEP '\\'
#define _dirname(c1) _splitpath(c1, NULL, c1, NULL, NULL)
#else
#define SEP_C "/"
#define SEP '/'
#include <libgen.h>
#define _dirname(c1) dirname(c1)
#endif

#define _MAX 260

char* readFile(char* path);
bool checkPath(char* filename);
char* real(char* p);

char* basename(char* path);
char* resolveLibrary(char* name);
void join(char* dest, const char* p1, const char* p2);
#endif