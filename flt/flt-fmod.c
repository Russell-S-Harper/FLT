/*
    Contents: "fmod" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#include "flt-tmp.h"

FLT flt_fmod(const FLT f, const FLT g) {
   FLT result;
   flt_tmp t, u, v;
   int sign;
   flt_to_tmp(&f, &t);
   flt_to_tmp(&g, &u);
   /* The sign of t "decides" the sign of the result */
   sign = t.s;
   t.s = u.s = 0;
   if (t.c == E_NORMAL && u.c == E_NORMAL &&
       flt_tmp_compare(&t, &u, E_GREATER_THAN_OR_EQUAL_TO)) {
      flt_tmp_copy(&v, &u);
      flt_tmp_invert(&u);
      flt_tmp_multiply(&t, &u);
      flt_tmp_modf(&t, &u);
      flt_tmp_multiply(&t, &v);
      t.s = sign;
      tmp_to_flt(&t, &result);
   } else {
      /* Default to NaN */
      result = FLT_NAN;
      switch (t.c) {
      case E_ZERO:
      case E_NORMAL:
         switch (u.c) {
         case E_INFINITE:
         case E_NORMAL:
            result = f;
            break;
         }
         break;
      }
   }
   return result;
}
