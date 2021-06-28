#ifndef pcrap_tools_h
#define pcrap_tools_h

#include <sys/unistd.h>
#include <string.h>

#include "common.h"
#include "memory.h"

#define _basename _splitpath

char* readFile(char* path);
bool checkPath(char* filename);

char* basename(char* path);
#endif