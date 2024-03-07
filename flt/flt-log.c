/*
    Contents: "log" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: russell.s.harper@gmail.com
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
		flt_tmp_init_log2(&u);
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
		flt_tmp_initialize(&u, E_NORMAL, 0, 0x4D104D42, -2); /* log10(2) = 0.301029995663981 */
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

#ifdef	POLY_LOG2
void flt_tmp_log2(flt_tmp *pt) {
	/* These correspond to the nonic polynomial:
		0.00538325528778299t^9 - 0.0815016079263423t^8 + 0.553726323899935t^7 - 2.2263874483398t^6
		+ 5.88190890593417t^5 - 10.7225754646689t^4 + 13.8105519024431t^3 - 12.7807736607554t^2
		+ 9.13914622920568t - 3.57947842907196
	*/
	static int s[] = {0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
	static uint32_t m[] = {0x5833025A, 0x53752852, 0x46E08112, 0x473E90E4, 0x5E1C4C83,
				0x55C7D5A5, 0x6E7C02A3, 0x663F0643, 0x491CF8B3, 0x728B1659};
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
		flt_tmp_init_0(pt);
	/* Approximate using the polynomial */
	else
		flt_tmp_evaluate(pt, sizeof(s) / sizeof(int), s, m, e);
	/* Add in the exponent */
	flt_tmp_initialize(&u, E_NORMAL, exponent < 0, abs(exponent), TMP_1_BITS);
	flt_tmp_normalize(&u);
	flt_tmp_add(pt, &u);
}
#else
void flt_tmp_log2(flt_tmp *pt) {
	/*	These correspond to CORDIC coefficients:
		m_r = 3/2, 5/4, 9/8, ... (exponents are all zero)
		m_i = 2/3, 4/5, 8/9, ... (exponents are all -1)
		m_l = log2(3/2), log2(5/4), log2(9/8), ... (exponents are -1 - index)
	*/
	static uint32_t m_r[] = {
		0x60000000, 0x50000000, 0x48000000, 0x44000000, 0x42000000, 0x41000000,
		0x40800000, 0x40400000, 0x40200000, 0x40100000, 0x40080000, 0x40040000,
		0x40020000, 0x40010000, 0x40008000, 0x40004000, 0x40002000, 0x40001000,
		0x40000800, 0x40000400, 0x40000200, 0x40000100, 0x40000080, 0x40000040,
		0x40000020, 0x40000010, 0x40000008, 0x40000004, 0x40000002, 0x40000001
	};
	static uint32_t m_i[] = {
		0x55555555, 0x66666666, 0x71C71C72, 0x78787878, 0x7C1F07C2, 0x7E07E07E,
		0x7F01FC08, 0x7F807F80, 0x7FC01FF0, 0x7FE007FE, 0x7FF00200, 0x7FF80080,
		0x7FFC0020, 0x7FFE0008, 0x7FFF0002, 0x7FFF8000, 0x7FFFC000, 0x7FFFE000,
		0x7FFFF000, 0x7FFFF800, 0x7FFFFC00, 0x7FFFFE00, 0x7FFFFF00, 0x7FFFFF80,
		0x7FFFFFC0, 0x7FFFFFE0, 0x7FFFFFF0, 0x7FFFFFF8, 0x7FFFFFFC, 0x7FFFFFFE
	};
	static uint32_t m_l[] = {
		0x4AE00D1D, 0x5269E12F, 0x570068E8, 0x598FDBEB, 0x5AEB4DD6, 0x5B9E5A17,
		0x5BF942DC, 0x5C2711B6, 0x5C3E0FFC, 0x5C4994DD, 0x5C4F58BE, 0x5C523B0B,
		0x5C53AC48, 0x5C5464EC, 0x5C54C140, 0x5C54EF6A, 0x5C55067F, 0x5C55120A,
		0x5C5517CF, 0x5C551AB2, 0x5C551C23, 0x5C551CDC, 0x5C551D38, 0x5C551D67,
		0x5C551D7E, 0x5C551D89, 0x5C551D8F, 0x5C551D92, 0x5C551D93, 0x5C551D94
	};
	flt_tmp u, v;
	int exponent, i, j;
	/* Input is expected to be normal and non-negative */
	if (pt->c != E_NORMAL || pt->s)
		exit(EXIT_FAILURE);
	/* Save the exponent and normalize 1 <= t < 2 */
	exponent = pt->e;
	pt->e = 0;
	/* Shortcut if mantissa = 1.0 */
	if (pt->m == TMP_1)
		flt_tmp_init_0(pt);
	/* Approximate using the CORDIC coefficients */
	else {
		flt_tmp_init_0(&u);
		for (i = 0, j = sizeof(m_r) / sizeof(uint32_t); pt->m > TMP_1 && !pt->e && i < j; ++i) {
			if (pt->m >= m_r[i] && !pt->e) {
				flt_tmp_initialize(&v, E_NORMAL, 0, m_i[i], -1);
				flt_tmp_multiply(pt, &v);
				flt_tmp_initialize(&v, E_NORMAL, 0, m_l[i], -1 - i);
				flt_tmp_add(&u, &v);
			}
		}
		flt_tmp_copy(pt, &u);
	}
	/* Add in the exponent */
	flt_tmp_initialize(&u, E_NORMAL, exponent < 0, abs(exponent), TMP_1_BITS);
	flt_tmp_normalize(&u);
	flt_tmp_add(pt, &u);
}
#endif	/* POLY_LOG2 */
