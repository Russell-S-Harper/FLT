/*
    Contents: "pow" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#include <stdlib.h>

#include "flt-tmp.h"

static FLT flt_tmp_pow_alt(flt_tmp *pt, flt_tmp *pu);
static void flt_tmp_powN(flt_tmp *pt, flt_tmp *pu);
static bool flt_tmp_is_odd(const flt_tmp *pt);

FLT flt_pow(const FLT f, const FLT g) {
	FLT result;
	flt_tmp t, u, v, w;
	flt_to_tmp(&f, &t);
	flt_to_tmp(&g, &u);
	/*
		pow(t, +/-0) returns 1 for any t, even when t is NaN
		pow(+1, u) returns 1 for any u, even when u is NaN
	*/
	flt_tmp_init_1(&v);
	if (u.c == E_ZERO || flt_tmp_compare(&t, &v, E_EQUAL_TO))
		return FLT_POS_1;
	/*
		pow(+Inf, u) returns +0 for any negative u
		pow(+Inf, u) returns +Inf for any positive u
	*/
	if (t.c == E_INFINITE && !t.s && u.c != E_NAN)
		return u.s? FLT_POS_0: FLT_POS_INF;
	/* Check for integer exponents */
	flt_tmp_copy(&v, &u);
	if (v.c == E_NORMAL)
		flt_tmp_modf(&v, &w);
	if (v.c == E_ZERO) {
		/* No remainder, so u is an integer, call powN */
		flt_tmp_powN(&t, &u);
		tmp_to_flt(&t, &result);
	} else if (t.c == E_NORMAL && !t.s) {
		/* Using the identity t^u = 2^(u*log2(t)) */
		flt_tmp_log2(&t);
		flt_tmp_multiply(&t, &u);
		if (t.c == E_NORMAL) {
			flt_tmp_exp2(&t);
			tmp_to_flt(&t, &result);
		} else
			result = flt_tmp_exp2_alt(&t);
	} else
		result = flt_tmp_pow_alt(&t, &u);
	return result;
}

/* Convenience function to handle pow special cases. Note +Inf^u, t^0, and 1^u are already handled. */
static FLT flt_tmp_pow_alt(flt_tmp *pt, flt_tmp *pu) {
	flt_tmp v;
	switch (pt->c) {
	case E_INFINITE:
		if (pt->s) {
			switch (pu->c) {
			/*
				pow(-Inf, u) returns +0 if u is a negative non-integer
				pow(-Inf, u) returns +Inf if u is a positive non-integer
			*/
			case E_INFINITE:
			case E_NORMAL:
				return pu->s? FLT_POS_0: FLT_POS_INF;
			}
			break;
		}
		break;
	case E_ZERO:
		switch (pu->c) {
		/*
			pow(+/-0, -Inf) returns +Inf
			pow(+/-0, +Inf) returns +0
			pow(+/-0, u), where u is a negative non-integer, returns +Inf
			pow(+/-0, u), where u is a positive non-integer, returns +0
		*/
		case E_INFINITE:
		case E_NORMAL:
			return pu->s? FLT_POS_INF: FLT_POS_0;
		}
		break;
	case E_NORMAL:
		switch (pu->c) {
		/*
			pow(-1, +/-Inf) returns 1
			pow(t, -Inf) returns +Inf for any |t|<1
			pow(t, -Inf) returns +0 for any |t|>1
			pow(t, +Inf) returns +0 for any |t|<1
			pow(t, +Inf) returns +Inf for any |t|>1
		*/
		case E_INFINITE:
			/* Compare t to -1 */
			flt_tmp_initialize(&v, E_NORMAL, 1, TMP_1, 0);
			if (flt_tmp_compare(pt, &v, E_EQUAL_TO))
				return FLT_POS_1;
			else {
				/* Use absolute value of t and v */
				pt->s = v.s = 0;
				return flt_tmp_compare(pt, &v, pu->s? E_LESS_THAN: E_GREATER_THAN)? FLT_POS_INF: FLT_POS_0;
			}
		}
		break;
	}
	/* NaN handles the remaining cases */
	return FLT_NAN;
}

/* To handle t^u where u is an integer. Note +Inf^u, t^0, and 1^u are already handled. */
static void flt_tmp_powN(flt_tmp *pt, flt_tmp *pu) {
	flt_tmp v, w;
	int sign;
	bool u_is_odd = flt_tmp_is_odd(pu);
	/* Continue */
	if (pt->c == E_NORMAL) {
		/* Convert negative exponent t^-u = (1/t)^u */
		if (pu->s) {
			pu->s = 0;
			flt_tmp_invert(pt);
		}
		/* Determine the sign of the result */
		sign = u_is_odd? pt->s: 0;
		pt->s = 0;
		/* Problem has now been reduced to t^u with t, u > 0 */
		flt_tmp_init_1(&v);
		while (v.c == E_NORMAL && pu->e >= 0) {
			/* If the exponent is odd, multiply in the base */
			if (flt_tmp_is_odd(pu))
				flt_tmp_multiply(&v, pt);
			/* Square the base */
			flt_tmp_multiply(pt, pt);
			/* Halve the exponent */
			pu->e -= 1;
		}
		flt_tmp_copy(pt, &v);
		/* Restore the sign */
		pt->s = sign;
	} else {
		/* Handle all the other cases */
		switch (pt->c) {
		case E_ZERO:
			if (pu->s)
				/*
					pow(+0, u), where u is a negative odd integer, returns +Inf
					pow(+0, u), where u is a positive odd integer, returns +0
					pow(-0, u), where u is a negative odd integer, returns -Inf
					pow(-0, u), where u is a positive odd integer, returns -0
				*/
				pt->c = E_INFINITE;
			if (!u_is_odd)
				/*
					pow(+/-0, u), where u is a negative even integer, returns +Inf
					pow(+/-0, u), where u is a positive even integer, returns +0
				*/
				pt->s = 0;
			break;
		case E_INFINITE:
			if (pu->s)
				/*
					pow(-Inf, u), where u is a negative odd integer, returns -0
					pow(-Inf, u), where u is a positive odd integer, returns -Inf
				*/
				pt->c = E_ZERO;
			if (!u_is_odd)
				/*
					pow(-Inf, u), where u is a negative even integer, returns +0
					pow(-Inf, u), where u is a positive even integer, returns +Inf
				*/
				pt->s = 0;
			break;
		default:
			/* Anything else, set to NaN */
			pt->c = E_NAN;
			break;
		}
	}
}

static bool flt_tmp_is_odd(const flt_tmp *pt) {
	bool result = false;
	flt_tmp u, v;
	flt_tmp_copy(&u, pt);
	if (u.c == E_NORMAL) {
		u.s = 0;
		/* Remove any existing decimals */
		flt_tmp_modf(&u, &v);
		/* Check if v/2 has a remainder */
		if (v.e >= 0) {
			v.e -= 1;	/* v/2 */
			flt_tmp_modf(&v, &u);
			/* Remainder means odd */
			result = (v.c == E_NORMAL);
		}
	}
	return result;
}
