/*
    Contents: "modf" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#include <stdlib.h>

#include "flt-tmp.h"

FLT flt_modf(const FLT f, FLT *pg) {
   FLT result;
   flt_tmp t, u;
   /* Destination can't be NULL */
   if (pg == NULL)
      exit(EXIT_FAILURE);
   flt_to_tmp(&f, &t);
   if (t.c == E_NORMAL) {
      flt_tmp_modf(&t, &u);
      tmp_to_flt(&u, pg);
      tmp_to_flt(&t, &result);
   } else
      *pg = result = f;
   return result;
}

/* Note not static because also used by pow, fmod, ceil, and floor */
void flt_tmp_modf(flt_tmp *pt, flt_tmp *pu) {
   int sign;
   flt_tmp v;
   sign = pt->s;
   pt->s = 0;
   if (pt->e <= TMP_1_BITS) {
      flt_tmp_initialize(pu, E_ZERO, 0, 0, 0);
      while (pt->c == E_NORMAL && pt->e >= 0) {
         flt_tmp_initialize(&v, E_NORMAL, 0, TMP_1, pt->e);
         flt_tmp_add(pu, &v);
         flt_tmp_negate(&v);
         flt_tmp_add(pt, &v);
      }
   } else {
      flt_tmp_copy(pu, pt);
      flt_tmp_initialize(pt, E_ZERO, 0, 0, 0);
   }
   pt->s = pu->s = sign;
}
