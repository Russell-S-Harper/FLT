/*
    Contents: "htrig" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#include "flt-tmp.h"

static FLT flt_tmp_sinh_alt(flt_tmp *pt);
static FLT flt_tmp_cosh_alt(flt_tmp *pt);
static FLT flt_tmp_tanh_alt(flt_tmp *pt);
static FLT flt_tmp_asinh_alt(flt_tmp *pt);
static FLT flt_tmp_acosh_alt(flt_tmp *pt);
static FLT flt_tmp_atanh_alt(flt_tmp *pt);

FLT flt_sinh(const FLT f) {
	FLT result;
	flt_tmp t, u;
	flt_to_tmp(&f, &t);
	/* sinh(t) = (e^t - e^-t)/2 = (2^kt - 2^-kt)/2, where k = 1/log(2) */
	if (t.c == E_NORMAL) {
		flt_tmp_init_1_log2(&u);	/* k */
		flt_tmp_multiply(&t, &u);	/* kt */
		flt_tmp_copy(&u, &t);
		flt_tmp_exp2(&t);		/* 2^kt = e^t */
		flt_tmp_negate(&u);		/* -kt */
		flt_tmp_exp2(&u);		/* 2^-kt = e^-t */
		flt_tmp_negate(&u);		/* -e^-t */
		flt_tmp_add(&t, &u);		/* e^t - e^-t */
		if (t.c == E_NORMAL)
			--t.e;			/* (e^t - e^-t)/2 */
		tmp_to_flt(&t, &result);
	} else
		result = flt_tmp_sinh_alt(&t);
	return result;
}

FLT flt_cosh(const FLT f) {
	FLT result;
	flt_tmp t, u;
	flt_to_tmp(&f, &t);
	/* cosh(t) = (e^t + e^-t)/2 = (2^kt + 2^-kt)/2, where k = 1/log(2) */
	if (t.c == E_NORMAL) {
		flt_tmp_init_1_log2(&u);	/* k */
		flt_tmp_multiply(&t, &u);	/* kt */
		flt_tmp_copy(&u, &t);
		flt_tmp_exp2(&t);		/* 2^kt = e^t */
		flt_tmp_negate(&u);		/* -kt */
		flt_tmp_exp2(&u);		/* 2^-kt = e^-t */
		flt_tmp_add(&t, &u);		/* e^t + e^-t */
		if (t.c == E_NORMAL)
			--t.e;			/* (e^t + e^-t)/2 */
		tmp_to_flt(&t, &result);
	} else
		result = flt_tmp_cosh_alt(&t);
	return result;
}

FLT flt_tanh(const FLT f) {
	FLT result;
	flt_tmp t, u, v;
	flt_to_tmp(&f, &t);
	/* tanh(t) = (e^t - e^-t)/(e^t + e^-t) = (2^kt - 2^-kt)/(2^kt + 2^-kt), where k = 1/log(2) */
	/* At flt_tmp precision, |tanh(t)| -> 1 for |t| >= 10 */
	flt_tmp_initialize(&u, E_NORMAL, t.s, 0x50000000, 3); /* 10.0 */
	if (t.c == E_NORMAL && flt_tmp_compare(&t, &u, t.s? E_GREATER_THAN: E_LESS_THAN)) {
		flt_tmp_init_1_log2(&u); 	/* k */
		flt_tmp_multiply(&t, &u);	/* kt */
		flt_tmp_copy(&u, &t);
		flt_tmp_exp2(&t);		/* 2^kt = e^t */
		flt_tmp_negate(&u);		/* -e^t */
		flt_tmp_exp2(&u);		/* 2^-kt = e^-kt */
		flt_tmp_copy(&v, &t);
		flt_tmp_add(&v, &u);		/* e^kt + e^-t */
		flt_tmp_invert(&v);		/* 1/(e^t + e^-t) */
		flt_tmp_negate(&u);		/* -e^-t */
		flt_tmp_add(&t, &u);		/* e^t - e^-t */
		flt_tmp_multiply(&t, &v);	/* (e^t - e^-t) / (e^t + e^-t) */
		tmp_to_flt(&t, &result);
	} else
		result = flt_tmp_tanh_alt(&t);
	return result;
}

FLT flt_asinh(const FLT f) {
	FLT result;
	flt_tmp t, u, v;
	int sign;
	flt_to_tmp(&f, &t);
	/* asinh(t) = log(t + sqrt(t^2 + 1)) = log(2)*log(t + sqrt(t^2 + 1)) */
	if (t.c == E_NORMAL) {
		/* The positive side is much more accurate, so use asinh(-t) = -asinh(t) for t < 0 */
		/* Save the sign and set t > 0 */
		sign = t.s;
		t.s = 0;
		flt_tmp_copy(&u, &t);		/* t */
		flt_tmp_multiply(&u, &t);	/* t^2 */
		flt_tmp_init_1(&v);
		flt_tmp_add(&u, &v);		/* t^2 + 1 */
		flt_tmp_sqrt_ext(&u);		/* sqrt(t^2 + 1) */
		flt_tmp_add(&t, &u);		/* t + sqrt(t^2 + 1) */
		flt_tmp_log2(&t);		/* log2(t + sqrt(t^2 + 1)) */
		flt_tmp_init_log2(&u);
		flt_tmp_multiply(&t, &u);	/* log(t + sqrt(t^2 + 1)) */
		/* Restore the sign */
		t.s = sign;
		tmp_to_flt(&t, &result);
	} else
		result = flt_tmp_asinh_alt(&t);
	return result;
}

FLT flt_acosh(const FLT f) {
	FLT result;
	flt_tmp t, u, v;
	flt_to_tmp(&f, &t);
	/* arcosh(t) = log(t + sqrt(t^2 - 1)) = log(2)*log(t + sqrt(t^2 - 1)) */
	/* t.e >= 0 is a quick check for t >= 1 */
	if (t.c == E_NORMAL && !t.s && t.e >= 0) {
		flt_tmp_copy(&u, &t);		/* t */
		flt_tmp_multiply(&u, &t);	/* t^2 */
		flt_tmp_initialize(&v, E_NORMAL, 1, TMP_1, 0);	/* -1.0 */
		flt_tmp_add(&u, &v);		/* t^2 - 1 */
		flt_tmp_sqrt_ext(&u);		/* sqrt(t^2 - 1) */
		flt_tmp_add(&t, &u);		/* t + sqrt(t^2 - 1) */
		flt_tmp_log2(&t);		/* log2(t + sqrt(t^2 - 1)) */
		flt_tmp_init_log2(&u);
		flt_tmp_multiply(&t, &u);	/* log(t + sqrt(t^2 - 1)) */
		tmp_to_flt(&t, &result);
	} else
		result = flt_tmp_acosh_alt(&t);
	return result;
}

/* The usable range of this function is between 0 <= |f| <= 0.999; above 0.999 the errors become increasingly significant. */
FLT flt_atanh(const FLT f) {
	FLT result;
	flt_tmp t, u, v;
	flt_to_tmp(&f, &t);
	/* arctanh(t) = log((1 + t)/(1 - t))/2 = log(2)*log((1 + t)/(1 - t))/2 */
	/* t.e < 0 is a quick check for t < 1 */
	if (t.c == E_NORMAL && t.e < 0) {
		flt_tmp_init_1(&u);
		flt_tmp_copy(&v, &t);		/* t */
		flt_tmp_negate(&v);		/* -t */
		flt_tmp_add(&u, &v);		/* 1 - t */
		flt_tmp_invert(&u);		/* 1/(1 - t) */
		flt_tmp_init_1(&v);
		flt_tmp_add(&t, &v);		/* 1 + t */
		flt_tmp_multiply(&t, &u);	/* (1 + t)/(1 - t) */
		flt_tmp_log2(&t);		/* log2((1 + t)/(1 - t)) */
		flt_tmp_initialize(&u, E_NORMAL, 0, TMP_2LOG2, -2); /* 2log(2)/4 = log(2)/2 */
		flt_tmp_multiply(&t, &u);	/* log((1 + t)/(1 - t))/2 */
		tmp_to_flt(&t, &result);
	} else
		result = flt_tmp_atanh_alt(&t);
	return result;
}

/* Convenience function to handle sinh special cases */
static FLT flt_tmp_sinh_alt(flt_tmp *pt) {
	switch (pt->c) {
	case E_INFINITE:
		return pt->s? FLT_NEG_INF: FLT_POS_INF;
	case E_NAN:
		return FLT_NAN;
	case E_ZERO:
		return pt->s? FLT_NEG_0: FLT_POS_0;
	}
}

/* Convenience function to handle cosh special cases */
static FLT flt_tmp_cosh_alt(flt_tmp *pt) {
	switch (pt->c) {
	case E_INFINITE:
		return FLT_POS_INF;
	case E_NAN:
		return FLT_NAN;
	case E_ZERO:
		return FLT_POS_1;
	}
}

/* Convenience function to handle tanh special cases */
static FLT flt_tmp_tanh_alt(flt_tmp *pt) {
	switch (pt->c) {
	case E_INFINITE:
	case E_NORMAL:
		return pt->s? FLT_NEG_1: FLT_POS_1;
	case E_NAN:
		return FLT_NAN;
	case E_ZERO:
		return pt->s? FLT_NEG_0: FLT_POS_0;
	}
}

/* Convenience function to handle asinh special cases */
static FLT flt_tmp_asinh_alt(flt_tmp *pt) {
	switch (pt->c) {
	case E_INFINITE:
		return pt->s? FLT_NEG_INF: FLT_POS_INF;
	case E_NAN:
		return FLT_NAN;
	case E_ZERO:
		return pt->s? FLT_NEG_0: FLT_POS_0;
	}
}

/* Convenience function to handle acosh special cases */
static FLT flt_tmp_acosh_alt(flt_tmp *pt) {
	switch (pt->c) {
	case E_INFINITE:
		return pt->s? FLT_NAN: FLT_POS_INF;
	case E_NAN:
	case E_NORMAL:
	case E_ZERO:
		return FLT_NAN;
	}
}

/* Convenience function to handle atanh special cases */
static FLT flt_tmp_atanh_alt(flt_tmp *pt) {
	switch (pt->c) {
	case E_INFINITE:
	case E_NAN:
		return FLT_NAN;
	case E_NORMAL:
		/* t = 1? */
		if (pt->m == TMP_1 && !pt->e)
			return pt->s? FLT_NEG_INF: FLT_POS_INF;
		else
			return FLT_NAN;
	case E_ZERO:
		return pt->s? FLT_NEG_0: FLT_POS_0;
	}
}
