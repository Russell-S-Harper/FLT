/*
    Contents: "basic" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#include "flt-tmp.h"

static uint32_t uint_multiply(const uint16_t a, const uint16_t b);

FLT flt_add(const FLT f, const FLT g) {
	FLT result;
	flt_tmp t, u;
	flt_to_tmp(&f, &t);
	flt_to_tmp(&g, &u);
	flt_tmp_add(&t, &u);
	tmp_to_flt(&t, &result);
	return result;
}

FLT flt_subtract(const FLT f, const FLT g) {
	FLT result;
	flt_tmp t, u;
	flt_to_tmp(&f, &t);
	flt_to_tmp(&g, &u);
	flt_tmp_negate(&u);
	flt_tmp_add(&t, &u);
	tmp_to_flt(&t, &result);
	return result;
}

FLT flt_multiply(const FLT f, const FLT g) {
	FLT result;
	flt_tmp t, u;
	flt_to_tmp(&f, &t);
	flt_to_tmp(&g, &u);
	flt_tmp_multiply(&t, &u);
	tmp_to_flt(&t, &result);
	return result;
}

FLT flt_divide(const FLT f, const FLT g) {
	FLT result;
	flt_tmp t, u;
	flt_to_tmp(&f, &t);
	flt_to_tmp(&g, &u);
	flt_tmp_invert(&u);
	flt_tmp_multiply(&t, &u);
	tmp_to_flt(&t, &result);
	return result;
}

FLT flt_add_into(FLT *pf, const FLT g) {
	flt_tmp t, u;
	if (!pf)
		return FLT_NAN;
	flt_to_tmp(pf, &t);
	flt_to_tmp(&g, &u);
	flt_tmp_add(&t, &u);
	tmp_to_flt(&t, pf);
	return *pf;
}

FLT flt_subtract_into(FLT *pf, const FLT g) {
	flt_tmp t, u;
	if (!pf)
		return FLT_NAN;
	flt_to_tmp(pf, &t);
	flt_to_tmp(&g, &u);
	flt_tmp_negate(&u);
	flt_tmp_add(&t, &u);
	tmp_to_flt(&t, pf);
	return *pf;
}

FLT flt_multiply_into(FLT *pf, const FLT g) {
	flt_tmp t, u;
	if (!pf)
		return FLT_NAN;
	flt_to_tmp(pf, &t);
	flt_to_tmp(&g, &u);
	flt_tmp_multiply(&t, &u);
	tmp_to_flt(&t, pf);
	return *pf;
}

FLT flt_divide_into(FLT *pf, const FLT g) {
	flt_tmp t, u;
	if (!pf)
		return FLT_NAN;
	flt_to_tmp(pf, &t);
	flt_to_tmp(&g, &u);
	flt_tmp_invert(&u);
	flt_tmp_multiply(&t, &u);
	tmp_to_flt(&t, pf);
	return *pf;
}

FLT flt_negated(const FLT f) {
	/* This seems wasteful given what little flt_tmp_negate does but we'll do it this way for consistency! */
	FLT result;
	flt_tmp t;
	flt_to_tmp(&f, &t);
	flt_tmp_negate(&t);
	tmp_to_flt(&t, &result);
	return result;
}

FLT flt_inverted(const FLT f) {
	FLT result;
	flt_tmp t;
	flt_to_tmp(&f, &t);
	flt_tmp_invert(&t);
	tmp_to_flt(&t, &result);
	return result;
}

void flt_tmp_normalize(flt_tmp *pt) {
	if (pt->c == E_NORMAL) {
		if (pt->m) {
			while (pt->m < TMP_1) {
				pt->m <<= 1;
				pt->e -= 1;
			}
			while (pt->m >= TMP_2) {
				pt->m >>= 1;
				++pt->e;
			}
		} else
			pt->c = E_ZERO;
	}
}

void flt_tmp_add(flt_tmp *pt, const flt_tmp *pu) {
	uint32_t am, bm;
	int ae, be;
	if (pt->c == E_NORMAL && pu->c == E_NORMAL) {
		/* Extract the mantissas and exponents */
		am = pt->m;
		ae = pt->e;
		bm = pu->m;
		be = pu->e;
		/* Align the quantities */
		while (bm && ae > be) {
			bm >>= 1;
			++be;
		}
		while (am && be > ae) {
			am >>= 1;
			++ae;
		}
		/* Check if adding or subtracting */
		if (pt->s == pu->s) {
			/* Adding */
			pt->m = am + bm;
			pt->e = am ? ae : be;
			flt_tmp_normalize(pt);
		} else {
			/* Subtracting */
			if (am >= bm)
				pt->m = am - bm;
			else {
				pt->m = bm - am;
				pt->s = pu->s;
			}
			/* Confirm non-zero result */
			if (pt->m) {
				pt->e = am ? ae : be;
				flt_tmp_normalize(pt);
			} else {
				pt->c = E_ZERO;
				pt->s = 0;
			}
		}
	} else {
		switch (pt->c) {
		case E_INFINITE:
			switch (pu->c) {
			case E_INFINITE:
				if (pt->s != pu->s)
					pt->c = E_NAN;
				break;
			case E_NAN:
				pt->c = pu->c;
				break;
			}
			break;
		case E_NORMAL:
			switch (pu->c) {
			case E_INFINITE:
				pt->s = pu->s;
			case E_NAN:
				pt->c = pu->c;
				break;
			}
			break;
		case E_ZERO:
			switch (pu->c) {
			case E_INFINITE:
				pt->s = pu->s;
			case E_NAN:
				pt->c = pu->c;
				break;
			case E_NORMAL:
				/* 0 + u = u, copy u to t */
				flt_tmp_copy(pt, pu);
				break;
			case E_ZERO:
				pt->s &= pu->s;
				break;
			}
			break;
		}
	}
}

void flt_tmp_multiply(flt_tmp *pt, const flt_tmp *pu) {
	flt_tmp v;
	uint16_t a1, a2, b1, b2;
	/* Handle a common case: t^2 = t * t */
	if (pu == pt) {
		flt_tmp_copy(&v, pu);
		pu = &v;
	}
	if (pt->c == E_NORMAL && pu->c == E_NORMAL) {
		/* Multiply */
		pt->s ^= pu->s;
		pt->e += pu->e;
		a1 = pt->m >> TMP_U_SHFT;
		a2 = pt->m & TMP_L_MASK;
		b1 = pu->m >> TMP_U_SHFT;
		b2 = pu->m & TMP_L_MASK;
		pt->m = (uint_multiply(a1, b1) << 2) + (uint_multiply(a1, b2) >> 14) +
				(uint_multiply(a2, b1) >> 14) + (uint_multiply(a2, b2) >> 30);
		flt_tmp_normalize(pt);
	} else {
		switch (pt->c) {
		case E_INFINITE:
			switch (pu->c) {
			case E_INFINITE:
			case E_NORMAL:
				pt->s ^= pu->s;
				break;
			case E_NAN:
			case E_ZERO:
				pt->c = E_NAN;
				break;
			}
			break;
		case E_NORMAL:
			switch (pu->c) {
			case E_INFINITE:
				pt->s ^= pu->s;
			case E_NAN:
			case E_ZERO:
				pt->c = pu->c;
				break;
			}
			break;
		case E_ZERO:
			switch (pu->c) {
			case E_INFINITE:
			case E_NAN:
				pt->c = E_NAN;
				break;
			case E_ZERO:
				pt->s ^= pu->s;
				break;
			}
			break;
		}
	}
}

void flt_tmp_negate(flt_tmp *pt) { pt->s ^= 1; }

void flt_tmp_invert(flt_tmp *pt) {
	uint32_t remainder, mantissa;
	int i;
	if (pt->c == E_NORMAL) {
		pt->e = -pt->e;
		/* Short circuit if mantissa = 1.0 */
		if (pt->m == TMP_1)
			return;
		/* Standard binary division */
		i = TMP_2_BITS;
		remainder = TMP_2;
		mantissa = pt->m;
		pt->m = 0;
		while (--i) {
			pt->m <<= 1;
			if (remainder >= mantissa) {
				++pt->m;
				remainder -= mantissa;
			}
			mantissa >>= 1;
		}
		flt_tmp_normalize(pt);
	} else {
		switch (pt->c) {
		case E_INFINITE:
			pt->c = E_ZERO;
			break;
		case E_ZERO:
			pt->c = E_INFINITE;
			break;
		}
	}
}

/* Optimized routine to multiply two 16-bit inputs to generate a 32-bit result */
static uint32_t uint_multiply(const uint16_t a, const uint16_t b) {
	uint32_t result = 0;
	uint16_t i = 0x8000;
	if (a && b) {
		while (i) {
			result <<= 1;
			if (i & b)
				result += a;
			i >>= 1;
		}
	}
	return result;
}
