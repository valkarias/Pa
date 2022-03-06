# Pcrap Update Log
## Sorting Lists
Some sorting functionality should've been implemented a long time ago, but here we are!  
 Sorting lists is finally possible and is provided by the *Sort* module (which is coded in Pcrap itself!)  

**{+}:** Remember to update Pcrap before importing it!    

Using it is as simple as doing:
```
use "Sort";
```  

### Functions
- quickSort
- selectionSort

**{+}:** The selection sort method was added for smaller arrays and to add a bit of variety  
since its a module, so expect more functions in the future.

## String Conversion
This means converting various types to a string type which is very useful in different situations.  
A new ``toString`` native function has been added that does exactly just that.  
It takes one argument which is the value and will return a string representation from it.

## Misc
- Removed  ``assertShow`` function and added one optional argument to ``assert`` to display a message instead.   
- Improved ``assert`` function to display neat and less confusing error messages  
```
//assert(false)
Assertion Failed: No Source

//assert(false, message)
Assertion Failed: message
```
- Added documentation for the Os module.