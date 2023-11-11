/*
    Contents: "ldexp" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#include "flt-tmp.h"

FLT flt_ldexp(const FLT f, const int exponent) {
   FLT result;
   flt_tmp t;
   flt_to_tmp(&f, &t);
   if (t.c == E_NORMAL)
      t.e += exponent;
   tmp_to_flt(&t, &result);
   return result;
}
