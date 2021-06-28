#include "tools.h"

bool checkPath(char* filename) {
  if (access(filename, F_OK) == 0) {
    return true;
  }

  return false;
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