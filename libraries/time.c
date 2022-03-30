#include "Pa_time.h"

//TODO: complete overhaul?

//
ObjLibrary* createTimeLibrary() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    ObjString* name = copyString("Time", 4);
    push(OBJ_VAL(name));
    ObjLibrary* library = newLibrary(name);
    push(OBJ_VAL(library));

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