/*
    Contents: "round" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#include "flt-tmp.h"

static void flt_tmp_ceil(flt_tmp *pt);
static void flt_tmp_floor(flt_tmp *pt);

FLT flt_round(const FLT f) {
	FLT result;
	flt_tmp t, u;
	flt_to_tmp(&f, &t);
	if (t.c == E_NORMAL) {
		flt_tmp_initialize(&u, E_NORMAL, t.s, TMP_1, -1);	/* +/- 0.5 */
		flt_tmp_add(&t, &u);
		flt_tmp_modf(&t, &u);
		tmp_to_flt(&u, &result);
	} else
		result = f;
	return result;
}

FLT flt_trunc(const FLT f) {
	FLT result;
	flt_tmp t, u;
	flt_to_tmp(&f, &t);
	if (t.c == E_NORMAL) {
		flt_tmp_modf(&t, &u);
		tmp_to_flt(&u, &result);
	} else
		result = f;
	return result;
}

FLT flt_ceil(const FLT f) {
	FLT result;
	flt_tmp t;
	int sign;
	flt_to_tmp(&f, &t);
	if (t.c == E_NORMAL) {
		sign = t.s;
		t.s = 0;
		if (sign)
			flt_tmp_floor(&t);
		else
			flt_tmp_ceil(&t);
		t.s = sign;
		tmp_to_flt(&t, &result);
	} else
		result = f;
	return result;
}

FLT flt_floor(const FLT f) {
	FLT result;
	flt_tmp t;
	int sign;
	flt_to_tmp(&f, &t);
	if (t.c == E_NORMAL) {
		sign = t.s;
		t.s = 0;
		if (sign)
			flt_tmp_ceil(&t);
		else
			flt_tmp_floor(&t);
		t.s = sign;
		tmp_to_flt(&t, &result);
	} else
		result = f;
	return result;
}

static void flt_tmp_ceil(flt_tmp *pt) {
	flt_tmp u;
	flt_tmp_copy(&u, pt);
	flt_tmp_modf(&u, pt);
	if (u.c == E_NORMAL) {
		flt_tmp_init_1(&u);
		flt_tmp_add(pt, &u);
	}
}

static void flt_tmp_floor(flt_tmp *pt) {
	flt_tmp u;
	flt_tmp_copy(&u, pt);
	flt_tmp_modf(&u, pt);
}
