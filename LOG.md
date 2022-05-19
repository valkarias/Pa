# Pa Update Log

### More Optimizations
Tail Calls are now properly implemented, now every single returned call expression inside a function is represented as a tail-call:
```
define b() {
    print("from b");
}

define a() {
    //tail-call
    return b();
}

a();
```
Its a waste to create a call-frame for `b()` so we just make use of `a`'s frame resulting in much better performance.  
Back then this optimization was only possible if some very specific conditions are met, but now, it works for everycase where the last return value of a function is another call to a function.  


**{+}:** Optimized the following opcodes:
- Addition `+`
- Division `/`
- Equality `==`
- Modulo Opcode `%`
- Power Opcode `**`

### Note
- The development on Pa itself is going to slow down, the reason being is that there is an IDE in the work in the aim to improve the tooling!