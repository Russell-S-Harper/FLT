/*
    Contents: "FLT" C header file (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#ifndef	_FLT_H
#define	_FLT_H

#include <stdbool.h>
#include <stdint.h>

/* IEEE 754 single-precision with 1 sign bit, 8 exponent bits, and 24 significand precision bits (23 explicitly stored) */
#ifndef	_FLT_TDF
#define	_FLT_TDF
typedef uint32_t FLT;
#endif	/* _FLT_TDF */

/* Predefined constants for +/-INFINITY, NAN, +/-ZERO, +/-ONE, and +/-PI/2 */
#define	FLT_POS_INF	(0x7F800000)
#define	FLT_NEG_INF	(0xFF800000)
#define	FLT_NAN		(0x7FFFFFFF)
#define	FLT_POS_0	(0x00000000)
#define	FLT_NEG_0	(0x80000000)
#define	FLT_POS_1	(0x3F800000)
#define	FLT_NEG_1	(0xBF800000)
#define	FLT_POS_PI_2	(0x3FC90FDB)	/* +PI/2 */
#define	FLT_NEG_PI_2	(0xBFC90FDB)	/* -PI/2 */

/* (Possibly) redefined "float.h" constants */
#undef	FLT_MIN
#define	FLT_MIN		(1.17549435082e-38)
#undef	FLT_TRUE_MIN
#define	FLT_TRUE_MIN	(1.40129846432e-45)
#undef	FLT_MAX
#define	FLT_MAX		(3.40282346639e+38)
#undef	FLT_EPSILON
#define	FLT_EPSILON	(1.19209289551e-07)

/* Floating point classes and comparisons */
typedef enum {E_INFINITE = 1, E_NAN, E_NORMAL, E_SUBNORMAL, E_ZERO} E_CLASS;
typedef enum {E_EQUAL_TO = 1, E_LESS_THAN, E_GREATER_THAN, E_LESS_THAN_OR_EQUAL_TO, E_GREATER_THAN_OR_EQUAL_TO, E_NOT_EQUAL_TO} E_COMPARE;

/* Used in *scanf, defined in flt_io.c */
extern int g_flt_last_scanf_result;

FLT flt_ldexp(const FLT f, const int exponent);
FLT flt_frexp(const FLT f, int *exponent);
FLT flt_modf(const FLT f, FLT *pg);
FLT flt_fmod(const FLT f, const FLT g);

FLT flt_fabs(const FLT f);
FLT flt_round(const FLT f);
FLT flt_trunc(const FLT f);
FLT flt_ceil(const FLT f);
FLT flt_floor(const FLT f);

FLT flt_sqrt(const FLT f);
FLT flt_log(const FLT f);
FLT flt_log10(const FLT f);
FLT flt_exp(const FLT f);
FLT flt_exp10(const FLT f);
FLT flt_pow(const FLT f, FLT g);

FLT flt_sin(const FLT f);
FLT flt_cos(const FLT f);
FLT flt_tan(const FLT f);

FLT flt_asin(const FLT f);
FLT flt_acos(const FLT f);
FLT flt_atan(const FLT f);
FLT flt_atan2(const FLT y, const FLT x);

FLT flt_sinh(const FLT f);
FLT flt_cosh(const FLT f);
FLT flt_tanh(const FLT f);

FLT flt_asinh(const FLT f);
FLT flt_acosh(const FLT f);
FLT flt_atanh(const FLT f);

E_CLASS flt_classify(const FLT f);
bool flt_isinf(const FLT f);
bool flt_isnan(const FLT f);
bool flt_isnormal(const FLT f);
bool flt_issubnormal(const FLT f);
bool flt_iszero(const FLT f);
bool flt_isfinite(const FLT f);
bool flt_compare(const FLT f, const FLT g, E_COMPARE compare);

FLT flt_add(const FLT f, const FLT g);
FLT flt_subtract(const FLT f, const FLT g);
FLT flt_multiply(const FLT f, const FLT g);
FLT flt_divide(const FLT f, const FLT g);
FLT flt_negated(const FLT f);
FLT flt_inverted(const FLT f);

FLT flt_add_into(FLT *pf, const FLT g);
FLT flt_subtract_into(FLT *pf, const FLT g);
FLT flt_multiply_into(FLT *pf, const FLT g);
FLT flt_divide_into(FLT *pf, const FLT g);

FLT flt_atof(const char *string);
const char *flt_ftoa(const FLT f, const char *format);
char *flt_get_scanf_buffer(int offset);
FLT flt_ltof(const int32_t a);
FLT flt_ultof(const uint32_t a);
int32_t flt_ftol(const FLT f);
uint32_t flt_ftoul(const FLT f);

FLT flt_fmin(const FLT f, const FLT g);
FLT flt_fmax(const FLT f, const FLT g);
int flt_fsgn(const FLT f);
FLT flt_hypot(const FLT f, const FLT g);
FLT flt_log2(const FLT f);
FLT flt_exp2(const FLT f);

#endif	/* _FLT_H */
