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
   /* t.e < 0 is a quick check for |t| < 1, and !t.e && t.m == TMP_1 is a quick
    * check for |t| = 1 */
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
   /* t.e < 0 is a quick check for |t| < 1, and !t.e && t.m == TMP_1 is a quick
    * check for |t| = 1 */
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
            flt_tmp_initialize(&v, E_NORMAL, 1, TMP_PI_2, 1); /* -PI */
            flt_tmp_add(&t, &v);
         } else
            /* x > 0, y < 0, atan2(y, x) = -atan(y/x) */
            flt_tmp_negate(&t);
      } else if (x_sign) {
         /* x < 0, y > 0, atan2(y, x) = PI - atan(y/x) */
         flt_tmp_initialize(&v, E_NORMAL, 0, TMP_PI_2, 1); /* PI */
         flt_tmp_negate(&t);
         flt_tmp_add(&t, &v);
      }
   } else if (t.c == E_ZERO && u.c == E_NORMAL) {
      /* Along X axis */
      if (x_sign)
         flt_tmp_initialize(&t, E_NORMAL, 0, TMP_PI_2, 1); /* PI */
      else
         flt_tmp_initialize(&t, E_ZERO, 0, 0, 0); /* 0 */
   } else if (t.c == E_NORMAL && u.c == E_ZERO)
      /* Along Y axis */
      flt_tmp_initialize(&t, E_NORMAL, y_sign, TMP_PI_2, 0); /* +/-PI/2 */
   else
      flt_tmp_initialize(&t, E_NAN, 0, 0, 0); /* NaN */
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
      return pt->s ? FLT_NEG_0 : FLT_POS_0;
   }
}

static void flt_tmp_sin(flt_tmp *pt) {
   /* These correspond to the nonic polynomial:
           0.00000191939248768592t^9 + 0.00000367850039052996t^8 -
      0.000206017385118935t^7 + 0.00000905377337007012t^6
           + 0.00832678456370062t^5 + 0.00000286491679624314t^4 -
      0.166667386624051t^3 + 0.0000000922267828359t^2
           + 0.999999995443107t + 0.00000000002710620617 */
   static int s[] = {0, 0, 1, 0, 0, 0, 1, 0, 0, 0};
   static uint32_t m[] = {0x406774B8, 0x7B6E13E8, 0x6C032F74, 0x4BF2D48C,
                          0x4436886C, 0x602172A9, 0x55556D7E, 0x63071AE3,
                          0x7FFFFFF6, 0x7736E000};
   static int e[] = {-19, -19, -13, -17, -7, -19, -3, -24, -1, -36};
   flt_tmp u;
   int sign;
   flt_tmp_fmod_2pi(pt);
   /* sin(-t) = -sin(t) */
   sign = pt->s;
   pt->s = 0;
   /* sin(t >= PI) = -sin(t - PI) */
   flt_tmp_initialize(&u, E_NORMAL, 0, TMP_PI_2, 1); /* PI */
   if (flt_tmp_compare(pt, &u, E_GREATER_THAN_OR_EQUAL_TO)) {
      u.s = 1;             /* -PI */
      flt_tmp_add(pt, &u); /* t - PI */
      sign ^= 1;
   }
   /* sin(t > PI/2) = sin(PI - t) */
   flt_tmp_initialize(&u, E_NORMAL, 0, TMP_PI_2, 0); /* PI/2 */
   if (flt_tmp_compare(pt, &u, E_GREATER_THAN)) {
      u.e = 1;             /* PI */
      pt->s = 1;           /* -t */
      flt_tmp_add(pt, &u); /* PI - t */
   }
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
           -0.0000025938914155233t^9 + 0.00003667027596855t^8 -
      0.000032378226725295t^7 - 0.00133294801582341t^6
           - 0.0000617994214713396t^5 + 0.0417092547798761t^4 -
      0.0000171843197505819t^3 - 0.499996430678849t^2
           - 0.0000002843402115928t + 1.0000000021811 */
   static int s[] = {1, 0, 1, 1, 1, 0, 1, 1, 1, 0};
   static uint32_t m[] = {0x57095B8C, 0x4CE7345A, 0x43E6ED99, 0x575B2823,
                          0x40CD27E8, 0x556BA96A, 0x481385D8, 0x7FFFC41E,
                          0x4C53B5E7, 0x40000002};
   static int e[] = {-19, -15, -15, -10, -14, -5, -16, -2, -22, 0};
   flt_tmp u;
   flt_tmp_fmod_2pi(pt);
   /* cos(t) = cos(-t) */
   pt->s = 0;
   /* cos(t > PI) = cos(2PI - t) */
   flt_tmp_initialize(&u, E_NORMAL, 0, TMP_PI_2, 1); /* PI */
   if (flt_tmp_compare(pt, &u, E_GREATER_THAN)) {
      u.e = 2;             /* 2PI */
      pt->s = 1;           /* -t */
      flt_tmp_add(pt, &u); /* 2PI - t */
   }
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
      return pt->s ? FLT_NEG_0 : FLT_POS_0;
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
      return pt->s ? FLT_NEG_0 : FLT_POS_0;
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
      return pt->s ? FLT_NEG_PI_2 : FLT_POS_PI_2;
   case E_NAN:
      return FLT_NAN;
   case E_ZERO:
      return pt->s ? FLT_NEG_0 : FLT_POS_0;
   }
}

static void flt_tmp_atan(flt_tmp *pt) {
   /* These correspond to the nonic polynomial:
           -0.0235777996395556t^9 + 0.0949127184056167t^8 -
      0.102028435265945t^7
      - 0.092973145710523t^6
           + 0.258007688741545t^5 - 0.0185360732062851t^4 - 0.33014442415698t^3
      - 0.00027111511805923t^2
           + 1.00000874908591t - 0.00000003401048775942 */
   static int s[] = {1, 0, 1, 1, 0, 1, 1, 1, 0, 1};
   static uint32_t m[] = {0x60931D66, 0x6130CCB6, 0x687A2463, 0x5F345A31,
                          0x420CCAB9, 0x4BEC7B43, 0x54845851, 0x47123A43,
                          0x400024B2, 0x490976A0};
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
