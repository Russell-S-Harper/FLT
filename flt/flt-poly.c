/*
    Contents: "poly" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#include "flt-tmp.h"

void flt_tmp_evaluate(flt_tmp *pt, const int limit, const int *ps,
                      const uint32_t *pm, const int *pe) {
    flt_tmp u, v;
    int i;
    /* Approximate using the polynomial */
    flt_tmp_initialize(&u, E_NORMAL, ps[0], pm[0], pe[0]);
    for (i = 1; i < limit; ++i) {
        flt_tmp_multiply(&u, pt);
        flt_tmp_initialize(&v, E_NORMAL, ps[i], pm[i], pe[i]);
        flt_tmp_add(&u, &v);
    }
    /* Save in t */
    flt_tmp_copy(pt, &u);
}
