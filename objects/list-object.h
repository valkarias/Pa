#ifndef pcrap_list_h
#define pcrap_list_h

#include "../src/object.h"
#include "../src/value.h"
#include "../src/vm.h"

void initListMethods();

//VM re-enterability issues.
//Written in PCrap instead for the sake of simplicity too.
#define LIST_EXTRA "//\n" \
"define repeat(list, action) {\n" \
"   assertShow(type(action) == \"function\", \"Argument must be a function from 'repeat'.\");\n" \
"\n"\
"   let len = list.length();\n" \
"   for let i = 0; i < len; i++ {\n" \
"       list[i] = action(list[i]);\n" \
"   }\n" \
"\n" \
"   return 0;\n" \
"}\n" \
"\n"\
"define slice(list, limit) {\n" \
"   assertShow(type(limit) == \"number\", \"Argument must be a number from 'slice'.\");\n" \
"\n"\
"   let temp = [];\n"\
"   if list.length() <= limit {\n" \
"       limit = list.length();\n" \
"   }\n" \
"\n"\
"   for let i = 0; i < limit; i++ {\n"\
"       temp.append(list[i]);\n"\
"   }\n" \
"\n"\
"   return temp;\n" \
"}\n" \
"\n" \

#endif