# Pa Update Log

### Optimizations everywhere!
- Compiler now cache constants! previously it would consume any constant it sees including already existing constants, this optimization prevents it from overflowing the constant max load.
- Arithmetic operations now support the ability to directly change the stack top to their results, previously these operations would perform multiple push and pop operations, now it only pops once then peeks the second value, then changes the top to be the result.
- the `not` operator (`!`) also supports this optimization.  

**{+}:** We are planning to improve the tooling of the language in the next updates or so, aiming to add features ranging from new modules and libraries to external tooling related to text editors and so on.

### Misc
- Minor changes to some example file
- Moved `time()` & `clock()` from the os module over to the time module.