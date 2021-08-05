#ifndef pcrap_list_h
#define pcrap_list_h

#include "../src/object.h"
#include "../src/value.h"
#include "../src/vm.h"

void initListMethods();

//VM re-enterability issues.
//Written in PCrap instead for the sake of simplicity too.
#define LIST_EXTRA "//\n" \
"define forEach(list, action) {\n" \
"   assertShow(type(action) == \"function\", \"Argument must be a function from 'forEach'.\");\n" \
"\n"\
"   let len = list.length();\n" \
"   for let i = 0; i < len; i++ {\n" \
"       list[i] = action(list[i]);\n" \
"   }\n" \
"\n" \
"   return 0;\n" \
"}\n" \
"\n" \

#endif