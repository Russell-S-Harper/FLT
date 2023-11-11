/*
    Contents: "classify" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#include "flt-tmp.h"

E_CLASS
flt_classify(const FLT f) {
	/* Get the mantissa and exponent */
	uint32_t temporary, mantissa, exponent;
	temporary = *(uint32_t *)&f;
	mantissa = temporary & 0x7FFFFF;
	exponent = (temporary >> 23) & 0xFF;
	switch (exponent) {
	case 0x00:
		return mantissa ? E_SUBNORMAL : E_ZERO;
	case 0xFF:
		return mantissa ? E_NAN : E_INFINITE;
	default:
		return E_NORMAL;
	}
}

bool flt_isinf(const FLT f) { return flt_classify(f) == E_INFINITE; }

bool flt_isnan(const FLT f) { return flt_classify(f) == E_NAN; }

bool flt_isnormal(const FLT f) { return flt_classify(f) == E_NORMAL; }

bool flt_issubnormal(const FLT f) { return flt_classify(f) == E_SUBNORMAL; }

bool flt_iszero(const FLT f) { return flt_classify(f) == E_ZERO; }

bool flt_isfinite(const FLT f) {
	E_CLASS c = flt_classify(f);
	return c == E_NORMAL || c == E_SUBNORMAL || c == E_ZERO;
}

/* Returns -1, 0, or +1 if f < 0, f = 0 (or NaN), or f > 0 respectively */
int flt_fsgn(const FLT f) {
	flt_tmp t;
	flt_to_tmp(&f, &t);
	if (t.c == E_ZERO || t.c == E_NAN)
		return 0;
	else
		return t.s ? -1 : +1;
}
