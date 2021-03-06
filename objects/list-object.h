#ifndef Pa_list_h
#define Pa_list_h

#include "../src/object.h"
#include "../src/value.h"
#include "../src/vm.h"

void initListMethods();

//VM re-enterability issues.
//Written in Pa instead for the sake of simplicity too.
#define LIST_EXTRA "//\n" \
"define repeat(list, action) {\n" \
"   assert type(action) == \"function\", \"Argument must be a function from 'repeat'.\";\n" \
"\n"\
"   let len = list.length();\n" \
"   for let i = 0; i < len; i++ {\n" \
"       action(list[i]);\n" \
"   }\n" \
"\n" \
"   return 0;\n" \
"}\n" \
"\n"\

#endif