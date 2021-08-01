#include <ctype.h>

#include "objects.h"
#include "../src/memory.h"

static Value lengthMethod(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'length()'.", argCount);
        return NOTCLEAR;
    }

    ObjString* string = AS_STRING(args[0]);
    return NUMBER_VAL(string->length);
}

static Value numberMethod(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'number()'.", argCount);
        return NOTCLEAR;
    } 

    char* numString = AS_CSTRING(args[0]);
    char* err;

    double num = strtod(numString, &err);

    if (err == numString) {
        runtimeError("A Conversion error occured on number()?!");
        return NOTCLEAR;
    }

    return NUMBER_VAL(num);
}

static Value splitMethod(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'split()'.", argCount);
        return NOTCLEAR;
    } 

    if (!IS_STRING(args[1])) {
        runtimeError("Argument must be a string from 'split()'.");
        return NOTCLEAR;
    }

    ObjString* st = AS_STRING(args[0]);
    char* dl = AS_CSTRING(args[1]);

    char* alloc = ALLOCATE(char, st->length + 1);
    char* ref = alloc;

    memmove(alloc, st->chars, st->length);
    alloc[st->length] = '\0';

    char* final;

    ObjList* l = newList();
    push(OBJ_VAL(l));

    if (strlen(dl) != 0) {
        do {
            final = strstr(alloc, dl);

            if (final) *final = '\0';

            Value objStr = OBJ_VAL(copyString(alloc, strlen(alloc)));

            push(objStr);
            appendToList(l, objStr);
            pop();

            alloc = final + strlen(dl);
        } while (final != NULL);
    }

    pop();
    FREE_ARRAY(char, ref, st->length + 1);
    return OBJ_VAL(l);
}

static Value lowerMethod(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'lower()'.", argCount);
        return NOTCLEAR;
    } 

    ObjString* string = AS_STRING(args[0]);
    char* alloc = ALLOCATE(char, string->length + 1);

    for (int i = 0; i < string->chars[i]; i++) {
        alloc[i] = tolower(string->chars[i]);
    }

    alloc[string->length] = '\0';

    return OBJ_VAL(takeString(alloc, string->length));
}

static Value upperMethod(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'upper()'.", argCount);
        return NOTCLEAR;
    } 

    ObjString* string = AS_STRING(args[0]);
    char* alloc = ALLOCATE(char, string->length + 1);

    for (int i = 0; i < string->chars[i]; i++) {
        alloc[i] = toupper(string->chars[i]);
    }

    alloc[string->length] = '\0';

    return OBJ_VAL(takeString(alloc, string->length));
}

static Value capitalizeMethod(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'capitalize()'.", argCount);
        return NOTCLEAR;
    } 

    ObjString* string = AS_STRING(args[0]);
    char* alloc = ALLOCATE(char, string->length + 1);

    memcpy(alloc, string->chars, string->length + 1);

    alloc[0] = toupper(string->chars[0]);
    alloc[string->length] = '\0';
    return OBJ_VAL(takeString(alloc, string->length));
}

static Value startsWithMethod(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'startsWith()'.", argCount);
        return NOTCLEAR;
    } 

    if (!IS_STRING(args[1])) {
        runtimeError("Argument must be a string from 'startsWith()'.");
        return NOTCLEAR;
    }

    ObjString* start = AS_STRING(args[1]);

    return BOOL_VAL(strncmp(AS_CSTRING(args[0]), start->chars, start->length) == 0);
}


static Value endswith(char* string, char* suffix)
{
    if (!string || !suffix) {
        return FALSE_VAL;
    }

    size_t lenstr = strlen(string);
    size_t lensuffix = strlen(suffix);
    
    if (lensuffix > lenstr)
        return FALSE_VAL;

    return BOOL_VAL(strncmp(string + lenstr - lensuffix, suffix, lensuffix) == 0);
}

static Value endsWithMethod(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'endsWith()'.", argCount);
        return NOTCLEAR;
    } 

    if (!IS_STRING(args[1])) {
        runtimeError("Argument must be a string from 'endsWith()'.");
        return NOTCLEAR;
    }

    ObjString* string = AS_STRING(args[0]);
    ObjString* suffix = AS_STRING(args[1]);

    return endswith(string->chars, suffix->chars);
}

static Value isAlphaMethod(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'isAlpha()'.", argCount);
        return NOTCLEAR;
    } 

    ObjString* string = AS_STRING(args[0]);

    if (string->length == 0) {
        return FALSE_VAL;
    }

    for (int i = 0; i < string->chars[i]; i++) {
        if (!isalpha(string->chars[i])) {
            return FALSE_VAL;
        }
    }

    return TRUE_VAL;
}

static Value isDigitMethod(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'isDigit()'.", argCount);
        return NOTCLEAR;
    } 

    ObjString* string = AS_STRING(args[0]);

    if (string->length == 0) {
        return FALSE_VAL;
    }

    for (int i = 0; i < string->chars[i]; i++) {
        if (!isdigit(string->chars[i])) {
            return FALSE_VAL;
        }
    }

    return TRUE_VAL;
}

static Value isSpaceMethod(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'isSpace()'.", argCount);
        return NOTCLEAR;
    } 

    ObjString* string = AS_STRING(args[0]);

    if (string->length == 0) {
        return FALSE_VAL;
    }

    for (int i = 0; i < string->chars[i]; i++) {
        if (!isspace(string->chars[i])) {
            return FALSE_VAL;
        }
    }

    return TRUE_VAL;
}

static Value trimSpaceMethod(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'trimSpace()'.", argCount);
        return NOTCLEAR;
    }

    ObjString* string = AS_STRING(args[0]);

    char* alloc = ALLOCATE(char, string->length + 1);
    memcpy(alloc, string->chars, string->length + 1);

    int i = 0;
    int j = 0;

    for (; i < string->length; i++, j++) {
        if (alloc[i] != ' ') {
            alloc[j] = string->chars[i];
        } else {
            j--;
        }
    }

    if (j != 0) {
        alloc = GROW_ARRAY(char, alloc, string->length + 1, j + 1); // shrink.
        string->length = j;
    }

    alloc[j] = '\0';
    return OBJ_VAL(takeString(alloc, string->length));
}

void replace_(char* src, char* str, char* rep) {
    char* p = strstr(src, str);

    if (p) {
        int repLen = strlen (rep);
        int len = strlen(src) + repLen - strlen(str);

        char r[len];
        memset (r, 0, len);

        if ( p >= src ) {
            strncpy (r, src, p-src);
            r[p - src] = '\0';

            memcpy(r, rep, repLen);
            strncat(r, p + strlen (str), p + strlen (str) - src + strlen(src));

            strcpy(src, r);
            replace_(p + repLen, str, rep);
        }
    }
}

//TODO: more testing.
static Value replaceMethod (int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d from 'replace()'.", argCount);
        return NOTCLEAR;
    } 

    if (!IS_STRING(args[1])) {
        runtimeError("First Argument must be a string from 'replace()'.");
        return NOTCLEAR;
    }

    if (!IS_STRING(args[2])) {
        runtimeError("Second Argument must be a string from 'replace()'.");
        return NOTCLEAR;
    }

    ObjString* string = AS_STRING(args[0]);

    char* alloc = ALLOCATE(char, string->length + 1);
    memcpy(alloc, string->chars, string->length + 1);

    ObjString* input = AS_STRING(args[1]);
    ObjString* output = AS_STRING(args[2]);

    alloc[string->length] = '\0';

    replace_(alloc, input->chars, output->chars);
    string->length = strlen(alloc);
    return OBJ_VAL(takeString(alloc, string->length));
}

//
void initStringMethods() {
    char* stringMethodStrings[] = {
        "number",
        "length",
        "split",

        "lower",
        "upper",

        "capitalize",

        "startsWith",
        "endsWith",

        "isAlpha",
        "isDigit",
        "isSpace",

        "trimSpace",
        "replace",
    };

    NativeFn stringMethods[] = {
        numberMethod,
        lengthMethod,
        splitMethod,

        lowerMethod,
        upperMethod,

        capitalizeMethod,

        startsWithMethod,
        endsWithMethod,

        isAlphaMethod,
        isDigitMethod,
        isSpaceMethod,

        trimSpaceMethod,
        replaceMethod,
    };

    for (uint8_t i = 0; i < sizeof(stringMethodStrings) / sizeof(stringMethodStrings[0]); i++) {
        defineNative(stringMethodStrings[i], stringMethods[i], &vm.stringNativeMethods);
    }
}