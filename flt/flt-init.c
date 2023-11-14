/*
    Contents: "init" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#include "flt-tmp.h"

void flt_tmp_initialize(flt_tmp *pt, const E_CLASS c, const int s, const uint32_t m, const int e) {
	pt->c = c;
	pt->s = s;
	pt->m = m;
	pt->e = e;
}

void flt_tmp_copy(flt_tmp *pt, const flt_tmp *pu) {
	pt->c = pu->c;
	pt->s = pu->s;
	pt->m = pu->m;
	pt->e = pu->e;
}

void flt_tmp_init_0(flt_tmp *pt) {
	pt->c = E_ZERO;
	pt->s = 0;
	pt->m = 0;
	pt->e = 0;
}

void flt_tmp_init_1(flt_tmp *pt) {
	pt->c = E_NORMAL;
	pt->s = 0;
	pt->m = TMP_1;
	pt->e = 0;
}

void flt_tmp_init_pi(flt_tmp *pt) {
	pt->c = E_NORMAL;
	pt->s = 0;
	pt->m = TMP_PI_2;
	pt->e = 1;
}

void flt_tmp_init_log2(flt_tmp *pt) {
	pt->c = E_NORMAL;
	pt->s = 0;
	pt->m = TMP_2LOG2;
	pt->e = -1;
}

void flt_tmp_init_1_log2(flt_tmp *pt) {
	pt->c = E_NORMAL;
	pt->s = 0;
	pt->m = TMP_1_LOG2;
	pt->e = 0;
}

void flt_tmp_init_nan(flt_tmp *pt) {
	pt->c = E_NAN;
	pt->s = 0;
	pt->m = 0;
	pt->e = 0;
}
