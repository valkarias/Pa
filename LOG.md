# Pcrap Update Log

### Assert as an Internal
- the old ``assert`` built-in function is now a fully fledged statement in Pcrap. Asserting is so basic that it was decided to  
be added to the language's core and this can be also considered as a micro optimization (cherry ontop!).  
All the external & internal built-in APIs got updated to this new change.  

The assert statement can be used as so
```
assert condition, "Optional Message/Context.";

assert false, "Error!";
assert [1,2,3].length() == 4;
```

### Micro-Optimizations
- The simple negate, increment and decrement opcodes now apply their changes directly to the values in the stack, previously these opcodes  
popped then changed and finally pushed which is a lot of operations for a simple change.

### Misc
- Fixed GC not collecting a class' methods.
- Minor improvements to display runtime errors.
- Fixed not freeing allocated resources incase of an import failure.
- Fixed private functions not being able to call themselves in their bodies.