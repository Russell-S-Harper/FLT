/*
    Contents: "parse" C header file (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#ifndef _FLT_TDF
#define _FLT_TDF
typedef struct {
  short h1, h2;
} FLT;
#endif /* _FLT_TDF */

/* To prevent inclusion of <math.h> and <float.h> */
#define _MATH_H
#define _FLOAT_H
