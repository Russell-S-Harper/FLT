/*
    Contents: "sqrt" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#include <stdlib.h>

#include "flt-tmp.h"

static FLT flt_tmp_sqrt_alt(flt_tmp *pt);

FLT flt_sqrt(const FLT f) {
	FLT result;
	flt_tmp t;
	flt_to_tmp(&f, &t);
	if (t.c == E_NORMAL && !t.s) {
		flt_tmp_sqrt(&t);
		tmp_to_flt(&t, &result);
	} else
		result = flt_tmp_sqrt_alt(&t);
	return result;
}

FLT flt_hypot(const FLT f, const FLT g) {
	FLT result;
	flt_tmp t, u;
	/* Using flt_tmp allows calculating sqrt(f^2 + g^2) without overflow */
	flt_to_tmp(&f, &t);
	flt_to_tmp(&g, &u);
	flt_tmp_multiply(&t, &t);	/* t^2 */
	flt_tmp_multiply(&u, &u);	/* u^2 */
	flt_tmp_add(&t, &u);		/* t^2 + u^2 */
	flt_tmp_sqrt_ext(&t);		/* sqrt(t^2 + u^2) */
	tmp_to_flt(&t, &result);
	return result;
}

/* Convenience function to handle sqrt special cases */
static FLT flt_tmp_sqrt_alt(flt_tmp *pt) {
	switch (pt->c) {
		case E_INFINITE:
			return pt->s? FLT_NAN: FLT_POS_INF;
		case E_NAN:
		case E_NORMAL:
			return FLT_NAN;
		case E_ZERO:
			return pt->s? FLT_NEG_0: FLT_POS_0;
	}
}

/* A more forgiving flt_tmp_sqrt; does a bit extra by handling t < 0 due to rounding errors, as well as handling t = 0, etc. */
void flt_tmp_sqrt_ext(flt_tmp *pt) {
	if (pt->c == E_NORMAL) {
		if (!pt->s)
			flt_tmp_sqrt(pt);
		else
			flt_tmp_init_0(pt); /* Forcing t < 0 to 0 to account for rounding errors */
	}
	/* All others leave t unmodified */
}

void flt_tmp_sqrt(flt_tmp *pt) {
	flt_tmp u, v;
	int exponent;
	int i;
	/* Input is expected to be normal and non-negative */
	if (pt->c != E_NORMAL || pt->s)
		exit(EXIT_FAILURE);
	/* Save the exponent and normalize 1 <= t < 2 */
	exponent = pt->e;
	pt->e = 0;
	/* Do only if mantissa is not 1 */
	if (pt->m > TMP_1) {
		/* Using Newton's method to find the square root */
		/* Initial guess is 0.411954965695194t + 0.600722632796001 */
		flt_tmp_copy(&u, pt);
		flt_tmp_initialize(&v, E_NORMAL, 0, 0x6975E171, -2); /* 0.411954965695194 */
		flt_tmp_multiply(pt, &v);
		flt_tmp_initialize(&v, E_NORMAL, 0, 0x4CE47AAF, -1); /* 0.600722632796001 */
		flt_tmp_add(pt, &v);			/* x */
		for (i = 0; i < 3; ++i) {
			flt_tmp_copy(&v, pt);		/* x */
			flt_tmp_invert(&v);		/* 1/x */
			flt_tmp_multiply(&v, &u);	/* t/x */
			flt_tmp_add(pt, &v);		/* x + t/x */
			--pt->e;			/* (x + t/x) / 2 */
		}
	}
	/* Handle odd exponents */
	if (exponent & 1) {
		flt_tmp_initialize(&u, E_NORMAL, 0, 0x5A82799A, 0); /* sqrt(2) = 1.4142135623731 */
		flt_tmp_multiply(pt, &u);
	}
	/* Restore half the exponent */
	pt->e += (exponent >> 1);
}
