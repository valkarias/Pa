#include "pcrap_math.h"

static Value absLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'abs()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number from 'abs()'.");
        return NOTCLEAR;
    }

    double number = AS_NUMBER(args[0]);

    if (number < 0) {
        return NUMBER_VAL(number * -1);
    }

    return NUMBER_VAL(number);
}

static Value floorLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'floor()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number from 'floor()'.");
        return NOTCLEAR;
    }

    double number = AS_NUMBER(args[0]);

    return NUMBER_VAL(floor(number));
}

static Value roundLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'round()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number from 'round()'.");
        return NOTCLEAR;
    }

    double number = AS_NUMBER(args[0]);

    return NUMBER_VAL(round(number));
}

static Value ceilLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'ceil()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number from 'ceil()'.");
        return NOTCLEAR;
    }

    double number = AS_NUMBER(args[0]);

    return NUMBER_VAL(ceil(number));
}

static Value logLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'log()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number from 'log()'.");
        return NOTCLEAR;
    }

    double number = AS_NUMBER(args[0]);

    double res = log(number);

    if (errno) {
        runtimeError("Math domain error from 'log()'.");
        return NOTCLEAR;
    }

    return NUMBER_VAL(res);
}

static Value expLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'exp()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number from 'exp()'.");
        return NOTCLEAR;
    }

    double number = AS_NUMBER(args[0]);

    return NUMBER_VAL(exp(number));
}

static Value sinLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'sin()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number from 'sin()'.");
        return NOTCLEAR;
    }

    double number = AS_NUMBER(args[0]);

    return NUMBER_VAL(sin(number));
}

static Value asinLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'asin()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number from 'asin()'.");
        return NOTCLEAR;
    }

    double number = AS_NUMBER(args[0]);

    double res = asin(number);

    if (errno) {
        runtimeError("Math domain error from 'asin()'.");
        return NOTCLEAR;
    }

    return NUMBER_VAL(res);
}

static Value cosLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'cos()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number from 'cos()'.");
        return NOTCLEAR;
    }

    double number = AS_NUMBER(args[0]);

    return NUMBER_VAL(cos(number));
}

static Value acosLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'acos()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number from 'acos()'.");
        return NOTCLEAR;
    }

    double number = AS_NUMBER(args[0]);
    double res = acos(number);

    if (errno) {
        runtimeError("Math domain error from 'acos()'.");
        return NOTCLEAR;
    }

    return NUMBER_VAL(res);
}

static Value tanLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'tan()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number from 'tan()'.");
        return NOTCLEAR;
    }

    double number = AS_NUMBER(args[0]);

    return NUMBER_VAL(tan(number));
}

static Value atanLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'atan()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number from 'atan()'.");
        return NOTCLEAR;
    }

    double number = AS_NUMBER(args[0]);

    return NUMBER_VAL(atan(number));
}

static Value sqrtLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'sqrt()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("Argument must be a number from 'sqrt()'.");
        return NOTCLEAR;
    }

    double number = AS_NUMBER(args[0]);

    if (number <= 0) {
        runtimeError("Argument must be bigger than 0 from 'sqrt()'.");
        return NOTCLEAR;
    }

    return NUMBER_VAL(sqrt(number));
}

static Value clampLib(int argCount, Value *args) {
    if (argCount != 3) {
        runtimeError("Expected 3 arguments but got %d from 'clamp()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("First Argument must be a number from 'clamp()'.");
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[1])) {
        runtimeError("Second Argument must be a number from 'clamp()'.");
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[2])) {
        runtimeError("ThirdArgument must be a number from 'clamp()'.");
        return NOTCLEAR;
    }

    // min, pref, max.
    return NUMBER_VAL( CLAMP( AS_NUMBER(args[0]), AS_NUMBER(args[1]), AS_NUMBER(args[2]) ) );
}

static Value minLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'min()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_LIST(args[0])) {
        runtimeError("Argument must be a list from 'min()'.");
        return NOTCLEAR;
    }

    ObjList* list = AS_LIST(args[0]);
    argCount = list->items.count;
    args = list->items.values;

    if (list->items.count == 0) {
        return CLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("List Should be all numeric from 'min()'.");
        return NOTCLEAR;
    }

    double min = AS_NUMBER(args[0]);

    for (int i = 1; i < argCount; ++i) {
        Value val = args[i];

        if (!IS_NUMBER(val)) {
            runtimeError("List Should be all numeric from 'min()'.");
            return NOTCLEAR;
        }

        double c = AS_NUMBER(val);

        if (min > c) {
            min = c;
        }
    }

    return NUMBER_VAL(min);
}

static Value maxLib(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("Expected 1 argument but got %d from 'max()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_LIST(args[0])) {
        runtimeError("Argument must be a list from 'max()'.");
        return NOTCLEAR;
    }

    ObjList* list = AS_LIST(args[0]);
    argCount = list->items.count;
    args = list->items.values;

    if (list->items.count == 0) {
        return CLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("List Should be all numeric from 'max()'.");
        return NOTCLEAR;
    }

    double max = AS_NUMBER(args[0]);

    for (int i = 1; i < argCount; ++i) {
        Value val = args[i];

        if (!IS_NUMBER(val)) {
            runtimeError("List Should be all numeric from 'max()'.");
            return NOTCLEAR;
        }

        double c = AS_NUMBER(val);

        if (max < c) {
            max = c;
        }
    }

    return NUMBER_VAL(max);
}

int gcd (int a, int b) {
    if ( a < 0 ) a = -a;
    if ( b < 0 ) b = -b;

    while ( b != 0 ) {
        a %= b;
        if ( a == 0 ) return b;

        b %= a;
    }

    return a;
}

static Value gcdLib(int argCount, Value *args) {
    if (argCount != 2) {
        runtimeError("Expected 2 arguments but got %d from 'gcd()'.", argCount);
        return NOTCLEAR;
    }

    if (!IS_NUMBER(args[0])) {
        runtimeError("First Argument must be a number from 'gcd()'.");
        return NOTCLEAR;
    }
    
    if (!IS_NUMBER(args[1])) {
        runtimeError("Second Argument must be a number from 'gcd()'.");
        return NOTCLEAR;
    }

    int a = AS_NUMBER(args[0]);
    int b = AS_NUMBER(args[1]);

    return NUMBER_VAL(gcd(a, b));
}

//
ObjLibrary* createMathLibrary() {
    ObjString* name = copyString("Math", 4);
    push(OBJ_VAL(name));
    ObjLibrary* library = newLibrary(name);
    push(OBJ_VAL(library));

    defineNative("abs", absLib, &library->values);
    defineNative("floor", floorLib, &library->values);
    defineNative("round", roundLib, &library->values);
    defineNative("ceil", ceilLib, &library->values);
    defineNative("log",  logLib, &library->values);
    defineNative("exp",  expLib, &library->values);

    defineNative("sqrt", sqrtLib, &library->values);
    defineNative("clamp", clampLib, &library->values);

    defineNative("sin", sinLib, &library->values);
    defineNative("cos", cosLib, &library->values);
    defineNative("tan", tanLib, &library->values);

    defineNative("asin", asinLib, &library->values);
    defineNative("acos", acosLib, &library->values);
    defineNative("atan", atanLib, &library->values);

    defineNative("min", minLib, &library->values);
    defineNative("max", maxLib, &library->values);

    defineNative("gcd", gcdLib, &library->values);

    defineProperty("pi", NUMBER_VAL(3.14159265358979), &library->values);
    defineProperty("e", NUMBER_VAL(2.71828182845905), &library->values);

    pop();
    pop();

    return library;
    
}