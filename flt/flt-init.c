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
