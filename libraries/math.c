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

    defineNative("sqrt", sqrtLib, &library->values);

    defineNative("sin", sinLib, &library->values);
    defineNative("cos", cosLib, &library->values);
    defineNative("tan", tanLib, &library->values);


    defineProperty("pi", NUMBER_VAL(3.14159265358979), &library->values);
    defineProperty("e", NUMBER_VAL(2.71828182845905), &library->values);

    pop();
    pop();

    return library;
    
}