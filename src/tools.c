#include "tools.h"

bool checkPath(char* filename) {
#if _WIN32 || __linux__ || __unix || __unix__
  if (access(filename, F_OK) == 0) {
    return true;
  }
#else
  if (fopen(filename, "r") != NULL) {
    return true;
  }
#endif

  return false;
}

void join(char* dest, const char* p1, const char* p2) {
  if (p1 && *p1) {
    int len = strlen(p1);
    strcpy(dest, p1);

    if (dest[len - 1] == SEP) {
      if (p2 && *p2) {
        strcpy(dest + len, (*p2 == SEP) ? (p2 + 1) : p2);
      }
    }
    else {
      if (p2 && *p2) {
        if (*p2 == SEP)
          strcpy(dest + len, p2);
        else {
          dest[len] = SEP;
          strcpy(dest + len + 1, p2);
        }
      }
    }
  }
  else if (p2 && *p2)
    strcpy(dest, p2);
  else
    dest[0] = '\0';
}

char* resolveLibrary(char* name) {
  char* apis_dir = malloc(sizeof(char) * _MAX);
  char* EXT = basename(name);

  // ???
  getcwd(apis_dir, _MAX);
  
  join(apis_dir, apis_dir, "libraries");
  join(apis_dir, apis_dir, "APIs");
  strcat(EXT, ".pc");
  join(apis_dir, apis_dir, EXT);
  //

  if (checkPath(apis_dir)) {
    return apis_dir;
  }

  return NULL;
}

char* basename(char* path) {
  char* bfname = ALLOCATE(char, strlen(path) + 1);
  memcpy(bfname, path, strlen(path));
  bfname[strlen(path)] = '\0';

#ifdef _WIN32
  _basename(bfname, NULL, NULL, bfname, NULL);
  return bfname;
#else
  return strrchr(bfname, '/');
#endif

  return NULL;
}

char* readFile(char* path) {
  FILE* file = fopen(path, "rb");
//> no-file
  if (file == NULL) {
    fprintf(stderr, "Could not open file \"%s\".\n", path);
    exit(74);
  }
//< no-file

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(fileSize + 1);
//> no-buffer
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
    exit(74);
  }
  
//< no-buffer
  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
//> no-read
  if (bytesRead < fileSize) {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
    exit(74);
  }
  
//< no-read
  buffer[bytesRead] = '\0';

  fclose(file);
  return buffer;
}