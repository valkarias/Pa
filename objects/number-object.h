#ifndef Pa_number_h
#define Pa_number_h

#include "../src/object.h"
#include "../src/value.h"
#include "../src/vm.h"

void initNumberMethods();

//VM re-enterability issues.
//Written in Pa instead for the sake of simplicity too.
#define NUMBER_EXTRA "//\n" \
"define repeat(num, action) {\n" \
"   assert type(action) == \"function\", \"Argument must be a function from 'repeat'.\";\n" \
"\n"\
"   for let i = 0; i < num; i++ {\n" \
"       action(i);\n" \
"   }\n" \
"\n" \
"   return 0;\n" \
"}\n" \
"\n"\

#endif