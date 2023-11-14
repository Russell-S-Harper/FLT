/*
    Contents: "compare" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#include "flt-tmp.h"

bool flt_compare(const FLT f, const FLT g, E_COMPARE compare) {
	flt_tmp t, u;
	flt_to_tmp(&f, &t);
	flt_to_tmp(&g, &u);
	return flt_tmp_compare(&t, &u, compare);
}

bool flt_tmp_compare(const flt_tmp *pt, const flt_tmp *pu, E_COMPARE compare) {
	flt_tmp v, w;
	/* IEEE 754 specifically dictates that +Infinity == +Infinity and -Infinity == -Infinity */
	if (pt->c == E_INFINITE && pu->c == E_INFINITE && pt->s == pu->s)
		return (compare == E_EQUAL_TO || compare == E_LESS_THAN_OR_EQUAL_TO || compare == E_GREATER_THAN_OR_EQUAL_TO);
	/* Everything else is handled by analyzing v - w */
	flt_tmp_copy(&v, pt);
	flt_tmp_copy(&w, pu);
	flt_tmp_negate(&w);
	flt_tmp_add(&v, &w);
	switch (v.c) {
		case E_NAN:
			return (compare == E_NOT_EQUAL_TO);
		case E_ZERO:
			return (compare == E_EQUAL_TO || compare == E_LESS_THAN_OR_EQUAL_TO || compare == E_GREATER_THAN_OR_EQUAL_TO);
		default:
			if (v.s)
				return (compare == E_LESS_THAN || compare == E_LESS_THAN_OR_EQUAL_TO || compare == E_NOT_EQUAL_TO);
			else
				return (compare == E_GREATER_THAN || compare == E_GREATER_THAN_OR_EQUAL_TO || compare == E_NOT_EQUAL_TO);
	}
}

FLT flt_fmin(const FLT f, const FLT g) {
	FLT result;
	flt_tmp t, u;
	flt_to_tmp(&f, &t);
	flt_to_tmp(&g, &u);
	/* NaN doesn't compare, can't be the min */
	if (t.c == E_NAN)
		result = g;
	else if (u.c == E_NAN)
		result = f;
	else
		result = flt_tmp_compare(&t, &u, E_LESS_THAN)? f: g;
	return result;
}

FLT flt_fmax(const FLT f, const FLT g) {
	FLT result;
	flt_tmp t, u;
	flt_to_tmp(&f, &t);
	flt_to_tmp(&g, &u);
	/* NaN doesn't compare, can't be the max */
	if (t.c == E_NAN)
		result = g;
	else if (u.c == E_NAN)
		result = f;
	else
		result = flt_tmp_compare(&t, &u, E_GREATER_THAN)? f: g;
	return result;
}
