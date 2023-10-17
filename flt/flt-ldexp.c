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
