# Pcrap Update Log

### Privacy & Access-Level
Pcrap now offers a brand new semantic which is access-level to modules' values as well as methods and attributes for instances & classes.  
which allows certain functionality to be hidden. This can be used to create proper Application Programming Interfaces.  
- This is explained in detail in the documentation website.

### Print
This deserves its very own section. The print keyword has been removed and replaced with a new fresh native built-in function which does exactly the same thing. The reason was behind the fact that it was very difficult to parse the keyword ``private`` and ``print`` seperately.  

It also got one more optional argument now,
the signature can be represented as ``print(value, end?)``. 
```
print("No new line", "");
```

### Bitwise Shifts & XOR
- Added bitwise shift operators & ``Exclusive Or`` and, they function as you expect!
```
25 << 5 //800
60 >> 2 //15

900 ^ 2 //902
```

### PBuild
PBuild no more uses ``git`` to get the repository downloaded into your computer, Instead it downloads it as a zipfile via an http request.  
It also extracts the zipfile and updates the paths to build it correctly.

### Misc
- The ``list.slice`` function now requires 2 arguments instead of 1 which represents ``list.slice(start, end)``.  
- New ``string.format``.
- Revamped & fixed the ``string.replace`` function.
- Refactors of the language's core code.

**{+}** Fixed Parsing of octal numbers.