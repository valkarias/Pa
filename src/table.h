//> Hash Tables table-h
#ifndef pcrap_table_h
#define pcrap_table_h

#include "common.h"
#include "value.h"
//> entry

#define TABLE_MAX_LOAD 0.75

typedef struct {
  ObjString* key;
  Value value;
} Entry;
//< entry

typedef struct {
  int count;
  int capacity;
  Entry* entries;
} Table;

//> init-table-h
void initTable(Table* table);

void freeTable(Table* table);

bool tableGet(Table* table, ObjString* key, Value* value);

bool tableSet(Table* table, ObjString* key, Value value);

bool tableDelete(Table* table, ObjString* key);

void tableAddAll(Table* from, Table* to);

ObjString* tableFindString(Table* table, const char* chars,
                           int length, uint32_t hash);

void tableRemoveWhite(Table* table);

void markTable(Table* table);

//< init-table-h
#endif
