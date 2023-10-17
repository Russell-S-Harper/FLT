/*
    Contents: "trig" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#include "flt-tmp.h"

static FLT flt_tmp_sin_alt(flt_tmp *pt);
static void flt_tmp_sin(flt_tmp *pt);
static FLT flt_tmp_cos_alt(flt_tmp *pt);
static void flt_tmp_cos(flt_tmp *pt);
static FLT flt_tmp_tan_alt(flt_tmp *pt);
static void flt_tmp_tan(flt_tmp *pt);
static FLT flt_tmp_asin_alt(flt_tmp *pt);
static void flt_tmp_asin(flt_tmp *pt);
static FLT flt_tmp_acos_alt(flt_tmp *pt);
static void flt_tmp_acos(flt_tmp *pt);
static FLT flt_tmp_atan_alt(flt_tmp *pt);
static void flt_tmp_atan(flt_tmp *pt);
static void flt_tmp_fmod_2pi(flt_tmp *pt);

FLT flt_sin(const FLT f) {
	FLT result;
	flt_tmp t;
	flt_to_tmp(&f, &t);
	if (t.c == E_NORMAL) {
		flt_tmp_sin(&t);		
		tmp_to_flt(&t, &result);
	} else
		result = flt_tmp_sin_alt(&t);
	return result;
}

FLT flt_cos(const FLT f) {
	FLT result;
	flt_tmp t;
	flt_to_tmp(&f, &t);
	if (t.c == E_NORMAL) {
		flt_tmp_cos(&t);		
		tmp_to_flt(&t, &result);
	} else
		result = flt_tmp_cos_alt(&t);
	return result;
}

FLT flt_tan(const FLT f) {
	FLT result;
	flt_tmp t;
	flt_to_tmp(&f, &t);
	if (t.c == E_NORMAL) {
		flt_tmp_tan(&t);		
		tmp_to_flt(&t, &result);
	} else
		result = flt_tmp_tan_alt(&t);
	return result;
}

FLT flt_asin(const FLT f) {
	FLT result;
	flt_tmp t;
	flt_to_tmp(&f, &t);
	/* t.e < 0 is a quick check for |t| < 1, and
		!t.e && t.m == TMP_1 is a quick check for |t| = 1 */
	if (t.c == E_NORMAL && (t.e < 0 || (!t.e && t.m == TMP_1))) {
		flt_tmp_asin(&t);		
		tmp_to_flt(&t, &result);
	} else
		result = flt_tmp_asin_alt(&t);
	return result;
}

FLT flt_acos(const FLT f) {
	FLT result;
	flt_tmp t;
	flt_to_tmp(&f, &t);
	/* t.e < 0 is a quick check for |t| < 1, and
		!t.e && t.m == TMP_1 is a quick check for |t| = 1 */
	if (t.c == E_NORMAL && (t.e < 0 || (!t.e && t.m == TMP_1))) {
		flt_tmp_acos(&t);		
		tmp_to_flt(&t, &result);
	} else
		result = flt_tmp_acos_alt(&t);
	return result;
}

FLT flt_atan(const FLT f) {
	FLT result;
	flt_tmp t;
	flt_to_tmp(&f, &t);
	if (t.c == E_NORMAL) {
		flt_tmp_atan(&t);		
		tmp_to_flt(&t, &result);
	} else
		result = flt_tmp_atan_alt(&t);
	return result;
}

FLT flt_atan2(const FLT y, const FLT x) {
	FLT result;
	flt_tmp t, u, v;
	int y_sign, x_sign;
	/* Note: mapping y -> t, x -> u */
	flt_to_tmp(&y, &t);
	flt_to_tmp(&x, &u);
	y_sign = t.s;
	x_sign = u.s;
	t.s = u.s = 0;
	if (t.c == E_NORMAL && u.c == E_NORMAL) {
		flt_tmp_invert(&u);
		flt_tmp_multiply(&t, &u);
		flt_tmp_atan(&t);
		if (y_sign) {
			if (x_sign) {
				/* x < 0, y < 0, atan2(y, x) = -PI + atan(y/x) */
				flt_tmp_initialize(&v, E_NORMAL, 1, TMP_PI_2, 1);	/* -PI */
				flt_tmp_add(&t, &v);
			} else
				/* x > 0, y < 0, atan2(y, x) = -atan(y/x) */
				flt_tmp_negate(&t);
		} else if (x_sign) {
			/* x < 0, y > 0, atan2(y, x) = PI - atan(y/x) */
			flt_tmp_initialize(&v, E_NORMAL, 0, TMP_PI_2, 1);		/* PI */
			flt_tmp_negate(&t);
			flt_tmp_add(&t, &v);
		}
	} else if (t.c == E_ZERO && u.c == E_NORMAL) {
		/* Along X axis */
		if (x_sign)
			flt_tmp_initialize(&t, E_NORMAL, 0, TMP_PI_2, 1);		/* PI */
		else
			flt_tmp_initialize(&t, E_ZERO, 0, 0, 0);				/* 0 */
	} else if (t.c == E_NORMAL && u.c == E_ZERO)
		/* Along Y axis */
		flt_tmp_initialize(&t, E_NORMAL, y_sign, TMP_PI_2, 0);		/* +/-PI/2 */
	else
		flt_tmp_initialize(&t, E_NAN, 0, 0, 0);						/* NaN */
	tmp_to_flt(&t, &result);
	return result;	
}

/* Constrain the argument to +/- 2PI */
static void flt_tmp_fmod_2pi(flt_tmp *pt) {
	flt_tmp u, v;
	int sign;
	sign = pt->s;
	pt->s = 0;
	flt_tmp_initialize(&u, E_NORMAL, 0, TMP_PI_2, 2); /* 4*PI/2 = 2*PI */
	flt_tmp_copy(&v, &u);
	flt_tmp_invert(&u);
	flt_tmp_multiply(pt, &u);
	flt_tmp_modf(pt, &u);
	flt_tmp_multiply(pt, &v);
	pt->s = sign;
}

/* Convenience function to handle sin special cases */
static FLT flt_tmp_sin_alt(flt_tmp *pt) {
	switch (pt->c) {
		case E_INFINITE:
		case E_NAN:
			return FLT_NAN;
		case E_ZERO:
			return pt->s? FLT_NEG_0: FLT_POS_0;
	}
}
									
static void flt_tmp_sin(flt_tmp *pt) {
	/* These correspond to the nonic polynomial:
		-0.00000215892011192075t^9 + 0.0000610420280696605t^8 - 0.000574181046317104t^7 + 0.00138096821267958t^6
		+ 0.00517151723684t^5 + 0.00445570722228601t^4 - 0.170315304695657t^3 + 0.00153095774193699t^2
		+ 0.999754290408447t + 0.00000381057198585086 */
	static int s[] = {1, 0, 1, 0, 0, 0, 1, 0, 0, 0};
	static uint32_t m[] = {0x4870FB88, 0x4001D83A, 0x4B4251A2, 0x5A80CD4F, 0x54BAEA5A,
							0x49009733, 0x5733914F, 0x6455356F, 0x7FF7F2D7, 0x7FDC906A};
	static int e[] = {-19, -14, -11, -10, -8, -8, -3, -10, -1, -19};
	int sign;
	flt_tmp_fmod_2pi(pt);
	/* sin(-t) = -sin(t) */
	sign = pt->s;
	pt->s = 0;
	/* Approximate using the polynomial */
	flt_tmp_evaluate(pt, sizeof(s) / sizeof(int), s, m, e);
	/* Update the sign */
	pt->s ^= sign;
}

/* Convenience function to handle cos special cases */
static FLT flt_tmp_cos_alt(flt_tmp *pt) {
	switch (pt->c) {
		case E_INFINITE:
		case E_NAN:
			return FLT_NAN;
		case E_ZERO:
			return FLT_POS_1;
	}
}

static void flt_tmp_cos(flt_tmp *pt) {
	/* These correspond to the nonic polynomial:
		-2.52930713705053E-017t^9 - 0.0000189045324418488t^8 + 0.000475122721920389t^7 - 0.00388275446033337t^6
		+ 0.00753837102678705t^5 + 0.0281875057309906^4 + 0.0136975100184024t^3 - 0.507055459000441t^2
		+ 0.00139731339368655t + 0.99996676067338 */
	static int s[] = {1, 1, 0, 1, 0, 0, 0, 1, 0, 0};
	static uint32_t m[] = {0x74A4C9C2, 0x4F4A964F, 0x7C8CF235, 0x7F3AE7B6, 0x7B823842,
							0x7374BDF4, 0x7035C2B2, 0x40E7317B, 0x5B930754, 0x7FFEE92B};
	static int e[] = {-56, -16, -12, -9, -8, -6, -7, -1, -10, -1};
	flt_tmp_fmod_2pi(pt);
	/* cos(t) = cos(-t) */
	pt->s = 0;
	/* Approximate using the polynomial */
	flt_tmp_evaluate(pt, sizeof(s) / sizeof(int), s, m, e);
}

/* Convenience function to handle tan special cases */
static FLT flt_tmp_tan_alt(flt_tmp *pt) {
	switch (pt->c) {
		case E_INFINITE:
		case E_NAN:
			return FLT_NAN;
		case E_ZERO:
			return pt->s? FLT_NEG_0: FLT_POS_0;
	}
}

static void flt_tmp_tan(flt_tmp *pt) {
	flt_tmp u;
	flt_tmp_fmod_2pi(pt);
	/* tan(t) = sin(t) / cos(t) */
	flt_tmp_copy(&u, pt);
	flt_tmp_sin(pt);
	flt_tmp_cos(&u);
	flt_tmp_invert(&u);
	flt_tmp_multiply(pt, &u);
}

/* Convenience function to handle asin special cases */
static FLT flt_tmp_asin_alt(flt_tmp *pt) {
	switch (pt->c) {
		case E_INFINITE:
		case E_NAN:
		case E_NORMAL:
			return FLT_NAN;
		case E_ZERO:
			return pt->s? FLT_NEG_0: FLT_POS_0;
	}
}

static void flt_tmp_asin(flt_tmp *pt) {
	flt_tmp u, v;
	int sign;
	/* asin(-t) = -asin(t) */
	sign = pt->s;
	pt->s = 0;
	/* asin(t) = atan(t/sqrt(1 - t^2)) */
	flt_tmp_copy(&v, pt);
	flt_tmp_multiply(&v, pt);
	flt_tmp_negate(&v);
	flt_tmp_initialize(&u, E_NORMAL, 0, TMP_1, 0);
	flt_tmp_add(&u, &v);
	flt_tmp_sqrt_ext(&u);
	flt_tmp_invert(&u);
	flt_tmp_multiply(pt, &u);
	flt_tmp_atan(pt);
	/* Update the sign */
	pt->s ^= sign;
}

/* Convenience function to handle acos special cases */
static FLT flt_tmp_acos_alt(flt_tmp *pt) {
	switch (pt->c) {
		case E_INFINITE:
		case E_NAN:
		case E_NORMAL:
			return FLT_NAN;
		case E_ZERO:
			return FLT_POS_PI_2;
	}
}

static void flt_tmp_acos(flt_tmp *pt) {
	flt_tmp u, v;
	int sign;
	/* acos(-t) = PI - acos(t) */
	sign = pt->s;
	pt->s = 0;
	/* acos(t) = atan(sqrt(1 - t^2)/t) */
	flt_tmp_copy(&v, pt);
	flt_tmp_multiply(&v, pt);
	flt_tmp_negate(&v);
	flt_tmp_initialize(&u, E_NORMAL, 0, TMP_1, 0);
	flt_tmp_add(&u, &v);
	flt_tmp_sqrt_ext(&u);
	flt_tmp_invert(pt);
	flt_tmp_multiply(pt, &u);
	flt_tmp_atan(pt);
	if (sign) {
		flt_tmp_negate(pt);
		flt_tmp_initialize(&u, E_NORMAL, 0, TMP_PI_2, 1); /* PI */
		flt_tmp_add(pt, &u);
	}
}

/* Convenience function to handle atan special cases */
static FLT flt_tmp_atan_alt(flt_tmp *pt) {
	switch (pt->c) {
		case E_INFINITE:
			return pt->s? FLT_NEG_PI_2: FLT_POS_PI_2;
		case E_NAN:
			return FLT_NAN;
		case E_ZERO:
			return pt->s? FLT_NEG_0: FLT_POS_0;
	}
}

static void flt_tmp_atan(flt_tmp *pt) {
	/* These correspond to the nonic polynomial:
		-0.0235777996395556t^9 + 0.0949127184056167t^8 - 0.102028435265945t^7 - 0.092973145710523t^6
		+ 0.258007688741545t^5 - 0.0185360732062851t^4 - 0.33014442415698t^3 - 0.00027111511805923t^2
		+ 1.00000874908591t - 0.00000003401048775942 */
	static int s[] = {1, 0, 1, 1, 0, 1, 1, 1, 0, 1};
	static uint32_t m[] = {0x60931D66, 0x6130CCB6, 0x687A2463, 0x5F345A31, 0x420CCAB9,
							0x4BEC7B43, 0x54845851, 0x47123A43, 0x400024B2, 0x490976A0};
	static int e[] = {-6, -4, -4, -4, -2, -6, -2, -12, 0, -25};
	flt_tmp u;
	int sign;
	/* atan(-t) = -atan(t) */
	sign = pt->s;
	pt->s = 0;
	/* Approximate using one of the polynomials */
	flt_tmp_initialize(&u, E_NORMAL, 0, TMP_1, 0); /* 1 */
	/* If less than or equal to one */
	if (flt_tmp_compare(pt, &u, E_LESS_THAN_OR_EQUAL_TO))
		flt_tmp_evaluate(pt, sizeof(s) / sizeof(int), s, m, e);
	/* If greater than one, use the identity tan(t) = PI/2 - (tan(1/t)) */
	else {
		flt_tmp_invert(pt);
		flt_tmp_evaluate(pt, sizeof(s) / sizeof(int), s, m, e);
		flt_tmp_negate(pt);
		flt_tmp_initialize(&u, E_NORMAL, 0, TMP_PI_2, 0); /* PI/2 */
		flt_tmp_add(pt, &u);
	}
	/* Restore sign */
	pt->s = sign;
}
