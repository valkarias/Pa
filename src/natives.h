#ifndef pcrap_natives_h
#define pcrap_natives_h

#define NOTCLEAR NIL_VAL
#define CLEAR NUMBER_VAL(0)

//meh
#define ERROR(msg) \
    do {\
        CallFrame* frame = &vm.frames[vm.frameCount - 1];\
        ObjFunction* function = frame->closure->function;\
        size_t instruction = frame->ip - function->chunk.code - 1;\
        fprintf(stderr, "%s\n[line %d] in '%s'.\n", msg, function->chunk.lines[instruction], function->library->name->chars);\
    } while(false)

void defineAllNatives();
#endif