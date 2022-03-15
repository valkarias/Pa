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

static Value toNumberMethod(int argCount, Value *args) {
    if (argCount != 0) {
        runtimeError("Expected 0 arguments but got %d from 'toNumber()'.", argCount);
        return NOTCLEAR;
    } 

    char* numString = AS_CSTRING(args[0]);
    char* err;

    double num = strtod(numString, &err);

    if (err == numString) {
        runtimeError("A Conversion error occured on 'toNumber()'?!");
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

static Value formatMethod (int argCount, Value *args) {
    if (argCount == 0) {
        runtimeError("Expected 1 or more arguments but got exactly 0 from 'format()'.", argCount);
        return NOTCLEAR;
    }

    int varLength = 0;
    //an array of characters (strings).
    char** variableStrings = ALLOCATE(char*, argCount);

    for (int arg = 1; arg < argCount + 1; arg++) {
        Value val = args[arg];
        if (!IS_STRING(val)) {
            // convert value to a string.
            variableStrings[arg - 1] = stringValue(val);
        } else {
            //otherwise, copy the original string.
            ObjString* object = AS_STRING(val);
            char* string = malloc(object->length + 1);
            memmove(string, object->chars, object->length + 1);
            variableStrings[arg - 1] = string;
        }

        varLength += strlen(variableStrings[arg - 1]);
    }

    ObjString* string = AS_STRING(args[0]);
    int sLen = string->length + 1;
    char* tmp = ALLOCATE(char, sLen);
    char* ref = tmp;
    memmove(tmp, string->chars, sLen);

    int count = 0;
    while ( (tmp = strstr(tmp, "{}"))) {
        count++;
        tmp++;
    }

    //reset
    tmp = ref;

    if (count != argCount) {
        runtimeError("Placeholders count must match the arguments from 'format()'.");
        
        //free variableStrings' content.
        for (int i = 0; i < argCount; ++i) {
            free(variableStrings[i]);
        }


        FREE_ARRAY(char, tmp, sLen);
        //free variableStrings itself.
        FREE_ARRAY(char*, variableStrings, argCount);
        return NOTCLEAR;
    } 

    //Dictu!
    int fLength = string->length - count * 2 + varLength + 1;
    //
    char* tmpPos;
    char* new = ALLOCATE(char, fLength);
    int stringLength = 0;

    for (int i = 0; i < argCount; ++i) {
        tmpPos = strstr(tmp, "{}");
        if (tmpPos != NULL)
            *tmpPos = '\0';
        
        int tmpLength = strlen(tmp);
        int rlen = strlen(variableStrings[i]);
        memmove(new + stringLength, tmp, tmpLength);
        memmove(new + stringLength + tmpLength, variableStrings[i], rlen);
        stringLength += tmpLength + rlen;
        tmp = tmpPos + 2;
        //no need to store them anymore since we already constructed our new string.
        free(variableStrings[i]);
    }

    FREE_ARRAY(char*, variableStrings, argCount);
    memmove(new + stringLength, tmp, strlen(tmp));
    new[fLength - 1] = '\0';
    FREE_ARRAY(char, ref, sLen);

    return OBJ_VAL(takeString(new, fLength - 1));
}

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

    Value value = args[0];
    ObjString* toReplace = AS_STRING(args[1]);
    ObjString* replace = AS_STRING(args[2]);
    char* string = AS_CSTRING(value);

    int count = 0;
    int len = toReplace->length;
    int rlen = replace->length;
    int sLen = strlen(string) + 1;

    char* tmp = ALLOCATE(char, sLen + 1);
    char* ref = tmp;
    memmove(tmp, string, sLen);
    tmp[sLen] = '\0';


    while ((tmp = strstr(tmp, toReplace->chars)) != NULL) {
        count++;
        tmp += len;
    }

    // reset
    tmp = ref;

    if (count == 0) {
        FREE_ARRAY(char, ref, sLen + 1);
        return value;
    }

    int length = strlen(tmp) - count * (len - rlen) + 1;
    char* tmpPos;
    char* new = ALLOCATE(char, length);
    int stringLength = 0;

    for (int i = 0; i < count; ++i) {
        tmpPos = strstr(tmp, toReplace->chars);
        if (tmpPos != NULL)
            *tmpPos = '\0';

        int tmpLength = strlen(tmp);
        memmove(new + stringLength, tmp, tmpLength);
        memmove(new + stringLength + tmpLength, replace->chars, rlen);
        stringLength += tmpLength + rlen;
        tmp = tmpPos + len;
    }

    memmove(new + stringLength, tmp, strlen(tmp));
    FREE_ARRAY(char, ref, sLen + 1);
    new[length - 1] = '\0';

    return OBJ_VAL(takeString(new, length - 1));
}

//
void initStringMethods() {
    char* stringMethodStrings[] = {
        "toNumber",
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
        "format"
    };

    NativeFn stringMethods[] = {
        toNumberMethod,
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
        formatMethod,
    };

    for (uint8_t i = 0; i < sizeof(stringMethodStrings) / sizeof(stringMethodStrings[0]); i++) {
        defineNative(stringMethodStrings[i], stringMethods[i], &vm.stringNativeMethods);
    }
}