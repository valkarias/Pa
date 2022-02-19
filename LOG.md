# Pcrap Update Log
## File i/o
File input & output is finally possible in pcrap via providing a new module that contains  
5 total functions and 3 properties which are:

### Functions
- open
- write
- read
- isEOF
- seek

### Properties
- SEEK_SET
- SEEK_CUR
- SEEK_END

**{+}:** Better explained in the up-coming documentation page for the *File* module.

## Compiler errors
A complete overhaul of the compiler errors, the current structure is not finalized BUT,
its better than before:
```
file.pc::line | where
   -> error message
```
**{+}:** Improved almost all of the errors' messages!

## Misc
Improved the description in the readme.  
Added The *Path* module documentation -> [seen here](https://valkarias.github.io/contents/chapters/stdlib/path.html).  
Added update logs from now on, to better express the new minor and major changes.