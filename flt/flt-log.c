/*
    Contents: "log" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#include <stdlib.h>

#include "flt-tmp.h"

static FLT flt_tmp_log2_alt(flt_tmp *pt);

FLT flt_log2(const FLT f) {
	FLT result;
	flt_tmp t;
	flt_to_tmp(&f, &t);
	if (t.c == E_NORMAL && !t.s) {
		flt_tmp_log2(&t);
		tmp_to_flt(&t, &result);
	} else
		result = flt_tmp_log2_alt(&t);
	return result;
}

FLT flt_log(const FLT f) {
	FLT result;
	flt_tmp t, u;
	flt_to_tmp(&f, &t);
	if (t.c == E_NORMAL && !t.s) {
		flt_tmp_log2(&t);
		/* 2log(2)/2 = log(2) */
		flt_tmp_initialize(&u, E_NORMAL, 0, TMP_2LOG2, -1);
		flt_tmp_multiply(&t, &u);
		tmp_to_flt(&t, &result);
	} else
		result = flt_tmp_log2_alt(&t);
	return result;
}

FLT flt_log10(const FLT f) {
	FLT result;
	flt_tmp t, u;
	flt_to_tmp(&f, &t);
	if (t.c == E_NORMAL && !t.s) {
		flt_tmp_log2(&t);
		/* log10(2) = 0.301029995663981 */
		flt_tmp_initialize(&u, E_NORMAL, 0, 0x4D104D42, -2);
		flt_tmp_multiply(&t, &u);
		tmp_to_flt(&t, &result);
	} else
		result = flt_tmp_log2_alt(&t);
	return result;
}

/* Convenience function to handle log2 special cases */
static FLT flt_tmp_log2_alt(flt_tmp *pt) {
	switch (pt->c) {
		case E_INFINITE:
			return pt->s? FLT_NAN: FLT_POS_INF;
		case E_NAN:
		case E_NORMAL:
			return FLT_NAN;
		case E_ZERO:
			return FLT_NEG_INF;
	}
}

void flt_tmp_log2(flt_tmp *pt) {
	/* These correspond to the nonic polynomial:
		0.00538325490414638t^9 - 0.0815016026617356t^8 + 0.55372629198425t^7 - 2.22638733617112t^6
		+ 5.88190865410353t^5 - 10.7225750901639t^4 + 13.8105515335726t^3 - 12.7807734287356t^2
		+ 9.13914614464219t - 3.57947841546609 */
	static int s[] = {0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
	static uint32_t m[] = {0x583301F1, 0x537527F8, 0x46E080CE, 0x473E90A8, 0x5E1C4C40,
							0x55C7D573, 0x6E7C0271, 0x663F0624, 0x491CF8A7, 0x728B1651};
	static int e[] = {-8, -4, -1, 1, 2, 3, 3, 3, 3, 1};
	flt_tmp u;
	int exponent;
	/* Input is expected to be normal and non-negative */
	if (pt->c != E_NORMAL || pt->s)
		exit(EXIT_FAILURE);
	/* Save the exponent and normalize 1 <= t < 2 */
	exponent = pt->e;
	pt->e = 0;
	/* Shortcut if mantissa = 1.0 */
	if (pt->m == TMP_1)
		flt_tmp_initialize(pt, E_ZERO, 0, 0, 0);
	/* Approximate using the polynomial */
	else
		flt_tmp_evaluate(pt, sizeof(s) / sizeof(int), s, m, e);
	/* Add in the exponent */
	flt_tmp_initialize(&u, E_NORMAL, (exponent < 0)? 1: 0, (exponent < 0)? -exponent: exponent, TMP_1_BITS);
	flt_tmp_normalize(&u);
	flt_tmp_add(pt, &u);
}
