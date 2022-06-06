#include "Pa_time.h"

#ifdef _WIN32
    #include "winbase.h"
#else
    #include <unistd.h>
#endif

static Value timeLib(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'time()'.", argCount);
        return NOTCLEAR;
    }

    return NUMBER_VAL( (double) time(NULL) );
}

static Value clockLib(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'clock()'.", argCount);
        return NOTCLEAR;
    }

    return NUMBER_VAL( (double)clock() / CLOCKS_PER_SEC );
}

static Value sleepLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'sleep()'.", argCount);
        return NOTCLEAR;
    }
    if (!IS_NUMBER(args[0])) {
        runtimeError("First argument must be a number from 'sleep()'.");
        return NOTCLEAR;
    }

    double time = AS_NUMBER(args[0]);
#ifdef _WIN32
    Sleep(time * 1000);
#else
    sleep(time);
#endif
    return CLEAR;
}

//
ObjLibrary* createTimeLibrary() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    ObjString* name = copyString("Time", 4);
    push(OBJ_VAL(name));
    ObjLibrary* library = newLibrary(name);
    push(OBJ_VAL(library));

    defineNative("time", timeLib, &library->values);
    defineNative("clock", clockLib, &library->values);
    defineNative("sleep", sleepLib, &library->values);

    defineProperty("MINYEAR", NUMBER_VAL(1), &library->values);
    defineProperty("MAXYEAR", NUMBER_VAL(9999), &library->values);

    defineProperty("day", NUMBER_VAL(tm.tm_mday), &library->values);
    defineProperty("weekday", NUMBER_VAL(tm.tm_wday), &library->values);

    defineProperty("month", NUMBER_VAL(tm.tm_mon + 1), &library->values);
    defineProperty("year", NUMBER_VAL(tm.tm_year + 1900), &library->values);

    defineProperty("hour", NUMBER_VAL(tm.tm_hour), &library->values);
    defineProperty("minute", NUMBER_VAL(tm.tm_min), &library->values);
    defineProperty("second", NUMBER_VAL(tm.tm_sec), &library->values);

    pop();
    pop();

    return library;
}