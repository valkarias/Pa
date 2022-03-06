#ifndef pcrap_natives_h
#define pcrap_natives_h

#define NOTCLEAR NIL_VAL
#define CLEAR NUMBER_VAL(0)

//meh
#define ERROR(msg) \
    do {\
        fprintf(stderr, "Assertion Failed: %s\n", msg);\
    } while(false)

void defineAllNatives();
#endif