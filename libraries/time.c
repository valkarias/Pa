#include "pcrap_time.h"

//i hate this.
//shrug.

static Value todayLib(int argCount, Value *args) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int size = 19;

    char* tstring = ALLOCATE(char, size + 1);

    snprintf(tstring , size,"%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    tstring[size + 1] = '\0';
    return OBJ_VAL(takeString(tstring, size));
}

static Value dateLib(int argCount, Value *args) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int size = 10;

    char* tstring = ALLOCATE(char, size + 1);

    snprintf(tstring , size,"%d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    tstring[size + 1] = '\0';
    return OBJ_VAL(takeString(tstring, size));
}

static Value timeLib(int argCount, Value *args) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int size = 8;

    char* tstring = ALLOCATE(char, size + 1);

    snprintf(tstring , size,"%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);

    tstring[size + 1] = '\0';
    return OBJ_VAL(takeString(tstring, size));
}


//
ObjLibrary* createTimeLibrary() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    ObjString* name = copyString("Time", 4);
    push(OBJ_VAL(name));
    ObjLibrary* library = newLibrary(name);
    push(OBJ_VAL(library));

    defineNative("today", todayLib, &library->values);
    defineNative("date", dateLib, &library->values);
    defineNative("time", timeLib, &library->values);

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