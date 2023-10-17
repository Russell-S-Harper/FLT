#include "flt-tmp.h"

FLT flt_fabs(const FLT f) {
	/* This seems wasteful given what little flt_tmp_abs
		does but we'll do it this way for consistency! */
	FLT result;
	flt_tmp t;
	flt_to_tmp(&f, &t);
	flt_tmp_abs(&t);
	tmp_to_flt(&t, &result);
	return result;
}

void flt_tmp_abs(flt_tmp *pt) {
	pt->s = 0;
}
