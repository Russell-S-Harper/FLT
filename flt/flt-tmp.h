/*
    Contents: "flt_tmp" C header file (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#ifndef	_FLT_TMP_H
#define	_FLT_TMP_H

#include <stdint.h>

#include "flt.h"

/* Internal structure with 7 bits more precision in the mantissa (vs. IEEE 754
 * single-precision) to use for calculations */
typedef struct {E_CLASS c; int s, e; uint32_t m;} flt_tmp;

/* Useful macros for flt_tmp */
#define	TMP_1_BITS	(30)
#define	TMP_1		(1L << (TMP_1_BITS))
#define	TMP_2_BITS	((TMP_1_BITS) + 1)
#define	TMP_2		(1L << (TMP_2_BITS))
#define	TMP_U_SHFT	(16)
#define	TMP_L_MASK	(0xFFFF)

/* Frequently used mantissas */
#define	TMP_PI_2	(0x6487ED51)	/* PI/2 = 1.5707963267949 */
#define	TMP_2LOG2	(0x58B90BFC)	/* 2*log(2) = 1.38629436111989 */
#define	TMP_1_LOG2	(0x5C551D95)	/* 1/log(2) = 1.44269504088896 */

void flt_to_tmp(const FLT *pf, flt_tmp *pt);
void tmp_to_flt(const flt_tmp *pt, FLT *pf);
void flt_tmp_initialize(flt_tmp *pt, const E_CLASS c, const int s, const uint32_t m, const int e);
void flt_tmp_init_0(flt_tmp *pt);
void flt_tmp_init_1(flt_tmp *pt);
void flt_tmp_copy(flt_tmp *pt, const flt_tmp *pu);
void flt_tmp_normalize(flt_tmp *pt);
bool flt_tmp_compare(const flt_tmp *pt, const flt_tmp *pu, E_COMPARE compare);
void flt_tmp_modf(flt_tmp *pt, flt_tmp *pu);
void flt_tmp_abs(flt_tmp *pt);
void flt_tmp_add(flt_tmp *pt, const flt_tmp *pu);
void flt_tmp_multiply(flt_tmp *pt, const flt_tmp *pu);
void flt_tmp_negate(flt_tmp *pt);
void flt_tmp_invert(flt_tmp *pt);
void flt_tmp_evaluate(flt_tmp *pt, const int limit, const int *ps, const uint32_t *pm, const int *pe);
void flt_tmp_sqrt(flt_tmp *pt);
void flt_tmp_sqrt_ext(flt_tmp *pt);
void flt_tmp_log2(flt_tmp *pt);
void flt_tmp_exp2(flt_tmp *pt);
FLT flt_tmp_exp2_alt(flt_tmp *pt);

#endif	/* _FLT_TMP_H */
