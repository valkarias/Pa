#ifndef pcrap_math_h
#define pcrap_math_h

#include <math.h>
#include <errno.h>

#include "../src/object.h"
#include "../src/value.h"
#include "../src/vm.h"

#include "library.h"

ObjLibrary* createMathLibrary();

//lol
#define MIN(A,B)    ({ __typeof__(A) __a = (A); __typeof__(B) __b = (B); __a < __b ? __a : __b; })
#define MAX(A,B)    ({ __typeof__(A) __a = (A); __typeof__(B) __b = (B); __a < __b ? __b : __a; })
//

//
#define CLAMP(low, x, high) ({\
  __typeof__(x) __x = (x); \
  __typeof__(low) __low = (low);\
  __typeof__(high) __high = (high);\
  __x > __high ? __high : (__x < __low ? __low : __x);\
  })

//

#endif
