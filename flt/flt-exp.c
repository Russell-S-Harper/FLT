#include <stdlib.h>

#include "flt-tmp.h"

static void flt_tmp_exp2_wrk(flt_tmp *pt);

FLT flt_exp2(const FLT f) {
	FLT result;
	flt_tmp t;
	flt_to_tmp(&f, &t);
	if (t.c == E_NORMAL) {
		flt_tmp_exp2(&t);
		tmp_to_flt(&t, &result);
	} else
		result = flt_tmp_exp2_alt(&t);
	return result;
}

FLT flt_exp(const FLT f) {
	FLT result;
	flt_tmp t, u;
	flt_to_tmp(&f, &t);
	if (t.c == E_NORMAL) {
		/* 1/log(2) */
		flt_tmp_initialize(&u, E_NORMAL, 0, TMP_1_LOG2, 0);
		flt_tmp_multiply(&t, &u);
		flt_tmp_exp2(&t);
		tmp_to_flt(&t, &result);
	} else
		result = flt_tmp_exp2_alt(&t);
	return result;
}

FLT flt_exp10(const FLT f) {
	FLT result;
	flt_tmp t, u;
	flt_to_tmp(&f, &t);
	if (t.c == E_NORMAL) {
		/* log(10)/log(2) = 3.32192809488736 */
		flt_tmp_initialize(&u, E_NORMAL, 0, 0x6A4D3C26, 1);
		flt_tmp_multiply(&t, &u);
		flt_tmp_exp2(&t);
		tmp_to_flt(&t, &result);
	} else
		result = flt_tmp_exp2_alt(&t);
	return result;
}

/* Convenience function to handle exp2 special cases, also used by pow */
FLT flt_tmp_exp2_alt(flt_tmp *pt) {
	switch (pt->c) {
		case E_INFINITE:
			return pt->s? FLT_POS_0: FLT_POS_INF;
		case E_NAN:
			return FLT_NAN;
		case E_ZERO:
			return FLT_POS_1;
	}
}

/* Normalizes if t < 0, then calls flt_tmp_exp2_wrk to do the real work */
void flt_tmp_exp2(flt_tmp *pt) {
	if (pt->s) {
		/* Convert 2^-t to 1/2^t */
		flt_tmp_negate(pt);
		flt_tmp_exp2_wrk(pt);
		flt_tmp_invert(pt);
	} else
		flt_tmp_exp2_wrk(pt);
}

/* Handles 2^t for t > 0 */
static void flt_tmp_exp2_wrk(flt_tmp *pt) {
	/* These correspond to the nonic polynomial:
		0.00000028872350637197t^9 - 0.0000001476563104036t^8 + 0.000021526314000785t^7 + 0.000137097976719918t^6
		+ 0.00136403718332427t^5 + 0.00958011749425546t^4 + 0.0555360142866787t^3 + 0.240209112077121t^2
		+ 0.693152748900943t + 0.999999204699729 */
	static int s[] = {0, 1, 0, 0, 0, 0, 0, 0, 0, 0};
	static uint32_t m[] = {0x4D80EDA4, 0x4F45BA91, 0x5A49B423, 0x47E0FA9C, 0x5964BF18,
							0x4E7AF66B, 0x71BCDDA9, 0x7AFCB051, 0x58B93AB2, 0x7FFFF954};
	static int e[] = {-22, -23, -16, -13, -10, -7, -5, -3, -1, -1};
	flt_tmp u;
	int i, exponent;
	/* Input is expected to be normal and non-negative */
	if (pt->c != E_NORMAL || pt->s)
		exit(EXIT_FAILURE);
	/* Handle out of range */
	flt_tmp_initialize(&u, E_NORMAL, 0, 0x4A800000, 7); /* 149.0 */
	if (flt_tmp_compare(pt, &u, E_GREATER_THAN)) {
		flt_tmp_initialize(pt, E_INFINITE, 0, 0, 0);
		return;
	}
	/* Save the exponent and normalize 1 <= t < 2 */
	exponent = pt->e;
	pt->e = 0;
	/* Shortcut if mantissa = 1.0 */
	if (pt->m == TMP_1)
		flt_tmp_initialize(pt, E_NORMAL, 0, TMP_1, 1);
	/* Approximate using the polynomial */
	else
		flt_tmp_evaluate(pt, sizeof(s) / sizeof(int), s, m, e);
	/* Positive powers */
	for (i = 0; pt->c == E_NORMAL && i < exponent; ++i) {
		flt_tmp_copy(&u, pt);
		flt_tmp_multiply(pt, &u);
	}
	/* Negative powers */
	for (i = 0; pt->c == E_NORMAL && i > exponent; --i)
		flt_tmp_sqrt(pt);
}
