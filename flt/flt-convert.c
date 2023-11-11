/*
    Contents: "convert" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#include "flt-tmp.h"

FLT flt_ltof(const int32_t a) {
   FLT result;
   flt_tmp t;
   if (a) {
      flt_tmp_initialize(&t, E_NORMAL, (a < 0) ? 1 : 0, (a < 0) ? -a : a,
                         TMP_1_BITS);
      flt_tmp_normalize(&t);
   } else
      flt_tmp_initialize(&t, E_ZERO, 0, 0, 0);
   tmp_to_flt(&t, &result);
   return result;
}

FLT flt_ultof(const uint32_t a) {
   FLT result;
   flt_tmp t;
   if (a) {
      flt_tmp_initialize(&t, E_NORMAL, 0, a, TMP_1_BITS);
      flt_tmp_normalize(&t);
   } else
      flt_tmp_initialize(&t, E_ZERO, 0, 0, 0);
   tmp_to_flt(&t, &result);
   return result;
}

int32_t flt_ftol(const FLT f) {
   /* Default handles a lot of cases! */
   int32_t result = 0;
   uint32_t mantissa;
   flt_tmp t, u;
   flt_to_tmp(&f, &t);
   /* Add one half for rounding */
   flt_tmp_initialize(&u, E_NORMAL, t.s, TMP_1, -1);
   flt_tmp_add(&t, &u);
   if (t.c == E_INFINITE || (t.c == E_NORMAL && t.e > TMP_1_BITS))
      result = t.s ? INT32_MIN : INT32_MAX;
   else if (t.c == E_NORMAL && t.e >= 0) {
      result = t.m >> (TMP_1_BITS - t.e);
      /* Banker's "un-rounding" */
      mantissa = t.m << (t.e + 2);
      if (!mantissa && (result & 1))
         --result;
      if (t.s)
         result = -result;
   }
   return result;
}

uint32_t flt_ftoul(const FLT f) {
   /* Default handles a lot of cases! */
   uint32_t result = 0, mantissa;
   flt_tmp t, u;
   flt_to_tmp(&f, &t);
   /* Add one half for rounding */
   flt_tmp_initialize(&u, E_NORMAL, t.s, TMP_1, -1);
   flt_tmp_add(&t, &u);
   if (t.c == E_INFINITE || (t.c == E_NORMAL && !t.s && t.e > TMP_2_BITS))
      result = UINT32_MAX;
   else if (t.c == E_NORMAL && !t.s && t.e >= 0) {
      result = (t.m << 1) >> (TMP_2_BITS - t.e);
      /* Banker's "un-rounding" */
      mantissa = t.m << (t.e + 2);
      if (!mantissa && (result & 1))
         --result;
   }
   return result;
}

void flt_to_tmp(const FLT *pf, flt_tmp *pt) {
   uint32_t temporary, mantissa;
   int sign, exponent;
   /* Get the sign, mantissa, and exponent */
   temporary = *(uint32_t *)pf;
   sign = temporary >> 31;
   mantissa = temporary & 0x7FFFFF;
   exponent = (temporary >> 23) & 0xFF;
   flt_tmp_initialize(pt, E_ZERO, 0, 0, 0);
   switch (pt->c = flt_classify(*pf)) {
   case E_INFINITE:
   case E_ZERO:
      pt->s = sign;
      break;
   case E_NORMAL:
      pt->s = sign;
      pt->m = TMP_1 + (mantissa << 7);
      pt->e = exponent - 127;
      break;
   case E_SUBNORMAL:
      /* flt_tmp can handle a wider range than FLT, so convert to normal */
      flt_tmp_initialize(pt, E_NORMAL, sign, mantissa, -119);
      flt_tmp_normalize(pt);
      break;
   }
}

void tmp_to_flt(const flt_tmp *pt, FLT *pf) {
   uint32_t sign, mantissa, exponent;
   int remainder;
   if (pt->c == E_NORMAL) {
      if (pt->e <= 127 && pt->e >= -126) {
         /* Normalize to IEEE 754 */
         sign = pt->s;
         exponent = pt->e + 127;
         mantissa = (pt->m >> 7) & 0x7FFFFF;
         /* Banker's rounding */
         remainder = pt->m & 0x7F;
         if (remainder > 0x40 || (remainder == 0x40 && (pt->m & 0x80)))
            ++mantissa;
         /* Check for overflow. If required, this will automatically roll to
          * +/- infinity */
         if (mantissa == 0x800000) {
            ++exponent;
            mantissa = 0;
         }
         *pf = (sign << 31) + (exponent << 23) + mantissa;
      } else if (pt->e > 127)
         /* Infinity */
         *pf = pt->s ? FLT_NEG_INF : FLT_POS_INF;
      else if (pt->e < -157)
         /* Zero */
         *pf = pt->s ? FLT_NEG_0 : FLT_POS_0;
      else {
         /* Subnormal - round up last 8 bits */
         sign = pt->s;
         mantissa = ((pt->m + 128) >> (-pt->e - 119));
         *pf = (sign << 31) + mantissa;
      }
   } else {
      switch (pt->c) {
      case E_INFINITE:
         *pf = pt->s ? FLT_NEG_INF : FLT_POS_INF;
         break;
      case E_NAN:
         *pf = FLT_NAN;
         break;
      case E_ZERO:
         *pf = pt->s ? FLT_NEG_0 : FLT_POS_0;
         break;
      }
   }
}
