/*
    Contents: "io" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "flt-tmp.h"

/* Modes when parsing strings */
typedef enum { E_BEGIN = 1, E_INTEGER, E_FRACTION, E_EXPONENT } E_MODE;

/* Rounding to use when converting to strings */
typedef enum { E_BEFORE = 1, E_AFTER } E_ROUNDING;

/* Maximum "E" format can be 17 characters including the null terminator:
 * +#.#########E+##_ */
#define FMT_E_MAX_LEN 17
#define FMT_E_BFR_LEN                                                          \
  (15 * FMT_E_MAX_LEN) /* Can provide up to 15 "E" calls in one printf         \
                          statement (15 * 17 = 255) */

/* Maximum "F" format can be 51 characters including the null terminator:
 * +#######################################.#########_ */
#define FMT_F_MAX_LEN 51
#define FMT_F_BFR_LEN                                                          \
  (5 * FMT_F_MAX_LEN) /* Can provide up to 5 "F" calls in one printf statement \
                         (5 * 51 = 255) */

/* Used in *scanf */
int g_flt_last_scanf_result;

static void flt_tmp_prepare_constants();
static const char *flt_tmp_e_format(flt_tmp *pt, const char *format);
static const char *flt_tmp_f_format(flt_tmp *pt, const char *format);
static const char *flt_tmp_g_format(flt_tmp *pt, const char *format);
static int flt_tmp_normalize_to_base_10(flt_tmp *pt, int precision,
                                        const E_ROUNDING rounding);
static char flt_tmp_get_next_digit(flt_tmp *pt);
static int printf_precision(const char *format);
static char *printf_post_process(char *string, const char *format,
                                 const int precision);

/* Constants frequently used in these routines */
static flt_tmp k1, k2, k3, k5, kn1, kn2, kn3, kn5, k10, k100, k1e3, k1e5, k1e8,
    k1en1, k1en2, k1en3, k1en5, k1en8, k5en1;

FLT flt_atof(const char *string) {
  FLT result;
  flt_tmp t, u, v, w;
  E_MODE mode;
  bool can_continue;
  int exponent, mantissa_sign, exponent_sign;
  int i;
  mode = E_BEGIN;
  can_continue = true;
  exponent = mantissa_sign = exponent_sign = 0;
  flt_tmp_prepare_constants();
  flt_tmp_initialize(&t, E_ZERO, 0, 0, 0);
  flt_tmp_copy(&v, &k1);
  flt_tmp_copy(&w, &k1);
  for (i = 0; can_continue && string[i]; ++i) {
    switch (tolower(string[i])) {
    case '+':
      switch (mode) {
      case E_INTEGER:
      case E_FRACTION:
        can_continue = false;
        break;
      }
      break;
    case '-':
      switch (mode) {
      case E_BEGIN:
        mantissa_sign ^= 1;
        break;
      case E_EXPONENT:
        if (!isdigit(string[i - 1]))
          exponent_sign ^= 1;
        else
          can_continue = false;
        break;
      default:
        can_continue = false;
        break;
      }
      break;
    case '.':
      switch (mode) {
      case E_BEGIN:
      case E_INTEGER:
        mode = E_FRACTION;
        break;
      default:
        can_continue = false;
        break;
      }
      break;
    case 'e':
      switch (mode) {
      case E_INTEGER:
      case E_FRACTION:
        mode = E_EXPONENT;
        break;
      default:
        can_continue = false;
        break;
      }
      break;
    case 'i':
      switch (mode) {
      case E_BEGIN:
        if (tolower(string[i + 1]) == 'n' && tolower(string[i + 2]) == 'f')
          flt_tmp_initialize(&t, E_INFINITE, mantissa_sign, 0, 0);
      default:
        can_continue = false;
      }
      break;
    case 'n':
      switch (mode) {
      case E_BEGIN:
        if (tolower(string[i + 1]) == 'a' && tolower(string[i + 2]) == 'n')
          flt_tmp_initialize(&t, E_NAN, 0, 0, 0);
      default:
        can_continue = false;
      }
      break;
    default:
      if (isdigit(string[i])) {
        switch (mode) {
        case E_BEGIN:
          mode = E_INTEGER;
        case E_INTEGER:
          flt_tmp_multiply(&t, &k10);
          if (string[i] > '0') {
            flt_tmp_initialize(&u, E_NORMAL, 0, string[i] - '0', TMP_1_BITS);
            flt_tmp_normalize(&u);
            flt_tmp_add(&t, &u);
          }
          break;
        case E_FRACTION:
          flt_tmp_multiply(&v, &k1en1);
          if (string[i] > '0') {
            flt_tmp_initialize(&u, E_NORMAL, 0, string[i] - '0', TMP_1_BITS);
            flt_tmp_normalize(&u);
            flt_tmp_multiply(&u, &v);
            flt_tmp_add(&t, &u);
          }
          break;
        case E_EXPONENT:
          exponent = 10 * exponent + string[i] - '0';
          break;
        }
        /* Allow leading spaces */
      } else if (isspace(string[i])) {
        switch (mode) {
        case E_INTEGER:
        case E_FRACTION:
        case E_EXPONENT:
          can_continue = false;
          break;
        }
        /* Stop if the character is not recognized */
      } else {
        if (mode == E_BEGIN)
          flt_tmp_initialize(&t, E_NAN, 0, 0, 0);
        can_continue = false;
      }
    }
  }
  /* Process for normal, zero, and infinity */
  if (t.c != E_NAN) {
    /* Process the sign */
    if (mantissa_sign)
      t.s = 1;
    /* For normal numbers */
    if (t.c == E_NORMAL) {
      /* Process the exponent */
      if (exponent_sign)
        exponent = -exponent;
      /* Quickly bring small exponents up */
      while (exponent < 0) {
        flt_tmp_multiply(&w, &k1en8);
        exponent += 8;
      }
      /* Quickly bring large exponents down */
      while (exponent >= 8) {
        flt_tmp_multiply(&w, &k1e8);
        exponent -= 8;
      }
      /* Use Fibonacci sequence to reduce the exponent to zero */
      if (exponent >= 5) {
        flt_tmp_multiply(&w, &k1e5);
        exponent -= 5;
      }
      if (exponent >= 3) {
        flt_tmp_multiply(&w, &k1e3);
        exponent -= 3;
      }
      if (exponent >= 2) {
        flt_tmp_multiply(&w, &k100);
        exponent -= 2;
      }
      if (exponent >= 1) {
        flt_tmp_multiply(&w, &k10);
        --exponent;
      }
      /* Scale the result */
      flt_tmp_multiply(&t, &w);
    }
  }
  /* Done */
  tmp_to_flt(&t, &result);
  return result;
}

const char *flt_ftoa(const FLT f, const char *format) {
  flt_tmp t;
  flt_to_tmp(&f, &t);
  if (strchr(format, 'e') || strchr(format, 'E'))
    return flt_tmp_e_format(&t, format);
  else if (strchr(format, 'f') || strchr(format, 'F'))
    return flt_tmp_f_format(&t, format);
  else
    return flt_tmp_g_format(&t, format);
}

static const char *flt_tmp_e_format(flt_tmp *pt, const char *format) {
  /* Formats recognized: /%\+?(\.[0-9])?[EeGg]/ */
  /* Using a larger buffer to allow multiple calls in one printf statement, i.e.
   * up to 15. */
  static char _strings[FMT_E_BFR_LEN];
  static int _index = 0;
  flt_tmp u;
  char *string;
  int base_10_exponent, precision, i, j;
  /* Advance the index */
  string = _strings + _index;
  _index = (_index + FMT_E_MAX_LEN) % FMT_E_BFR_LEN;
  /* Continue */
  i = 0;
  precision = printf_precision(format);
  if (pt->c == E_NORMAL) {
    flt_tmp_prepare_constants();
    flt_tmp_initialize(&u, pt->c, 0, pt->m, pt->e);
    /* Initial sign */
    string[i++] = "+-"[pt->s];
    /* Mantissa */
    base_10_exponent = flt_tmp_normalize_to_base_10(&u, precision, E_AFTER);
    /* First digit */
    string[i++] = flt_tmp_get_next_digit(&u);
    string[i++] = '.';
    /* Rest of the digits */
    for (j = 0; j < precision; ++j)
      string[i++] = flt_tmp_get_next_digit(&u);
    /* Exponent */
    string[i++] = 'e';
    string[i++] = (base_10_exponent < 0) ? '-' : '+';
    if (base_10_exponent < 0)
      base_10_exponent = -base_10_exponent;
    string[i++] = '0' + base_10_exponent / 10;
    string[i++] = '0' + base_10_exponent % 10;
    /* Done */
    string[i] = '\0';
  } else {
    switch (pt->c) {
    case E_INFINITE:
      strcpy(string, pt->s ? "-inf" : "+inf");
      break;
    case E_NAN:
      strcpy(string, "nan");
      break;
    case E_ZERO:
      strcpy(string, pt->s ? "-0." : "+0.");
      strncat(string, "000000000", precision);
      strcat(string, "e+00");
      break;
    }
  }
  return printf_post_process(string, format, precision);
}

static const char *flt_tmp_f_format(flt_tmp *pt, const char *format) {
  /* Formats recognized: /%\+?(\.[0-9])?[FfGg]/ */
  /* Using a larger buffer to allow multiple calls in one printf statement, i.e.
   * up to 5. */
  static char _strings[FMT_F_BFR_LEN];
  static int _index = 0;
  flt_tmp u;
  char *string;
  int base_10_exponent, precision, limit, i, j;
  /* Advance the index */
  string = _strings + _index;
  _index = (_index + FMT_F_MAX_LEN) % FMT_F_BFR_LEN;
  /* Continue */
  i = 0;
  precision = printf_precision(format);
  if (pt->c == E_NORMAL) {
    flt_tmp_prepare_constants();
    flt_tmp_initialize(&u, pt->c, 0, pt->m, pt->e);
    /* Initial sign */
    string[i++] = "+-"[pt->s];
    base_10_exponent = flt_tmp_normalize_to_base_10(&u, precision, E_BEFORE);
    /* Numbers one or higher */
    if (base_10_exponent >= 0) {
      /* Integer */
      string[i++] = flt_tmp_get_next_digit(&u);
      for (j = 0; j < base_10_exponent; ++j)
        string[i++] = flt_tmp_get_next_digit(&u);
      string[i++] = '.';
      /* Fraction */
      for (j = 0; j < precision; ++j)
        string[i++] = flt_tmp_get_next_digit(&u);
      /* Numbers less than one */
    } else {
      string[i++] = '0';
      string[i++] = '.';
      for (j = 0, limit = -base_10_exponent - 1; j < limit; ++j)
        string[i++] = '0';
      for (j = 0, limit = precision + base_10_exponent + 1; j < limit; ++j)
        string[i++] = flt_tmp_get_next_digit(&u);
    }
    /* Done */
    string[i] = '\0';
  } else {
    switch (pt->c) {
    case E_INFINITE:
      strcpy(string, pt->s ? "-inf" : "+inf");
      break;
    case E_NAN:
      strcpy(string, "nan");
      break;
    case E_ZERO:
      strcpy(string, pt->s ? "-0." : "+0.");
      strncat(string, "000000000", precision);
      break;
    }
  }
  return printf_post_process(string, format, precision);
}

static const char *flt_tmp_g_format(flt_tmp *pt, const char *format) {
  flt_tmp u;
  flt_tmp_prepare_constants();
  flt_tmp_initialize(&u, pt->c, 0, pt->m, pt->e);
  return flt_tmp_compare(&u, &k1e5, E_GREATER_THAN_OR_EQUAL_TO) ||
                 flt_tmp_compare(&u, &k1en5, E_LESS_THAN)
             ? flt_tmp_e_format(pt, format)
             : flt_tmp_f_format(pt, format);
}

static int flt_tmp_normalize_to_base_10(flt_tmp *pt, int precision,
                                        const E_ROUNDING rounding) {
  int base_10_exponent = 0;
  flt_tmp u;
  /* Input is expected to be normal and non-negative */
  if (pt->c != E_NORMAL || pt->s)
    exit(EXIT_FAILURE);
  /*
          Get rounding, i.e. start with half and divide down by the precision.
     The reason we're not using Banker's rounding here is because in testing,
     Banker's rounding rarely made a difference. Most of the time the input is
     multiplied by one or more of k1en8, k1en5, k1en3, k1en2, or k1en1 which
     don't have "clean" representations in base 2, so the input rarely has
     terminating 0s. Not worth the trouble for the few cases where it might
     apply!
  */
  flt_tmp_copy(&u, &k5en1);
  if (precision >= 5) {
    flt_tmp_multiply(&u, &k1en5);
    precision -= 5;
  }
  if (precision >= 3) {
    flt_tmp_multiply(&u, &k1en3);
    precision -= 3;
  }
  if (precision >= 2) {
    flt_tmp_multiply(&u, &k1en2);
    precision -= 2;
  }
  if (precision >= 1) {
    flt_tmp_multiply(&u, &k1en1);
    precision -= 1;
  }
  /* Add rounding before, if specified */
  if (rounding == E_BEFORE)
    flt_tmp_add(pt, &u);
  /* Quickly bring small numbers up */
  while (flt_tmp_compare(pt, &k1, E_LESS_THAN)) {
    flt_tmp_multiply(pt, &k1e8);
    base_10_exponent -= 8;
  }
  /* Quickly bring large numbers down */
  while (flt_tmp_compare(pt, &k1e8, E_GREATER_THAN_OR_EQUAL_TO)) {
    flt_tmp_multiply(pt, &k1en8);
    base_10_exponent += 8;
  }
  /* Normalize to 1 <= t < 10 */
  if (flt_tmp_compare(pt, &k1e5, E_GREATER_THAN_OR_EQUAL_TO)) {
    flt_tmp_multiply(pt, &k1en5);
    base_10_exponent += 5;
  }
  if (flt_tmp_compare(pt, &k1e3, E_GREATER_THAN_OR_EQUAL_TO)) {
    flt_tmp_multiply(pt, &k1en3);
    base_10_exponent += 3;
  }
  if (flt_tmp_compare(pt, &k100, E_GREATER_THAN_OR_EQUAL_TO)) {
    flt_tmp_multiply(pt, &k1en2);
    base_10_exponent += 2;
  }
  if (flt_tmp_compare(pt, &k10, E_GREATER_THAN_OR_EQUAL_TO)) {
    flt_tmp_multiply(pt, &k1en1);
    ++base_10_exponent;
  }
  /* Add rounding after, if specified */
  if (rounding == E_AFTER)
    flt_tmp_add(pt, &u);
  /* Last check for rounding issues */
  if (flt_tmp_compare(pt, &k10, E_GREATER_THAN_OR_EQUAL_TO)) {
    flt_tmp_multiply(pt, &k1en1);
    ++base_10_exponent;
  }
  if (flt_tmp_compare(pt, &k1, E_LESS_THAN)) {
    flt_tmp_multiply(pt, &k10);
    --base_10_exponent;
  }
  /* Return the base 10 exponent */
  return base_10_exponent;
}

static char flt_tmp_get_next_digit(flt_tmp *pt) {
  char digit = '0';
  /* Short circuit for zero */
  if (pt->c == E_ZERO)
    return digit;
  /* Use the Fibonacci sequence to extract the digit in fewer operations */
  if (flt_tmp_compare(pt, &k5, E_GREATER_THAN_OR_EQUAL_TO)) {
    flt_tmp_add(pt, &kn5);
    digit += 5;
  }
  if (flt_tmp_compare(pt, &k3, E_GREATER_THAN_OR_EQUAL_TO)) {
    flt_tmp_add(pt, &kn3);
    digit += 3;
  }
  if (flt_tmp_compare(pt, &k2, E_GREATER_THAN_OR_EQUAL_TO)) {
    flt_tmp_add(pt, &kn2);
    digit += 2;
  }
  if (flt_tmp_compare(pt, &k1, E_GREATER_THAN_OR_EQUAL_TO)) {
    flt_tmp_add(pt, &kn1);
    ++digit;
  }
  /* Advance to the next digit */
  flt_tmp_multiply(pt, &k10);
  /* Done */
  return digit;
}

/* Convenience function to return numeric precision */
static int printf_precision(const char *format) {
  char *decimal_marker = strchr(format, '.');
  /* Default to 6 if no decimal marker */
  return decimal_marker && isdigit(decimal_marker[1]) ? decimal_marker[1] - '0'
                                                      : 6;
}

/* Convenience function to post-process depending on format and precision */
static char *printf_post_process(char *string, const char *format,
                                 const int precision) {
  char *decimal_marker;
  int i;
  /* Convert to uppercase if required */
  if (strchr(format, 'E') || strchr(format, 'F') || strchr(format, 'G')) {
    for (i = 0; string[i]; ++i)
      string[i] = toupper(string[i]);
  }
  /* Remove the decimal marker if precision is zero */
  if (!precision && (decimal_marker = strchr(string, '.')))
    memmove(decimal_marker, decimal_marker + 1, strlen(decimal_marker + 1) + 1);
  /* Strip leading plus if required */
  if (!strchr(format, '+') && string[0] == '+')
    memmove(string, string + 1, strlen(string + 1) + 1);
  /* Done */
  return string;
}

char *flt_get_scanf_buffer(int offset) {
  /* Using a larger buffer to allow multiple calls in one scanf statement, i.e.
   * up to 5. */
  /* Assumes the worst case of the longest possible legal input! */
  static char _strings[FMT_F_BFR_LEN];
  int index = (FMT_F_MAX_LEN * abs(offset)) % FMT_F_BFR_LEN;
  /* A negative offset means get the buffer for writing into, so initialize to
   * NaN */
  if (offset < 0)
    strcpy(_strings + index, "nan");
  return _strings + index;
}

static void flt_tmp_prepare_constants() {
  static bool _prepared = false;
  FLT f;
  if (!_prepared) {
    /* Fibonacci 1, 2, 3, 5 */
    f = flt_ltof(1);
    flt_to_tmp(&f, &k1);
    f = flt_ltof(2);
    flt_to_tmp(&f, &k2);
    f = flt_ltof(3);
    flt_to_tmp(&f, &k3);
    f = flt_ltof(5);
    flt_to_tmp(&f, &k5);
    /* As above negated */
    flt_tmp_copy(&kn1, &k1);
    flt_tmp_negate(&kn1);
    flt_tmp_copy(&kn2, &k2);
    flt_tmp_negate(&kn2);
    flt_tmp_copy(&kn3, &k3);
    flt_tmp_negate(&kn3);
    flt_tmp_copy(&kn5, &k5);
    flt_tmp_negate(&kn5);
    /* 10, 100, 1e3, 1e5, 1e8 */
    f = flt_ltof(10);
    flt_to_tmp(&f, &k10);
    f = flt_ltof(100);
    flt_to_tmp(&f, &k100);
    f = flt_ltof(1000);
    flt_to_tmp(&f, &k1e3);
    f = flt_ltof(100000);
    flt_to_tmp(&f, &k1e5);
    f = flt_ltof(100000000);
    flt_to_tmp(&f, &k1e8);
    /* As above inverted */
    flt_tmp_copy(&k1en1, &k10);
    flt_tmp_invert(&k1en1);
    flt_tmp_copy(&k1en2, &k100);
    flt_tmp_invert(&k1en2);
    flt_tmp_copy(&k1en3, &k1e3);
    flt_tmp_invert(&k1en3);
    flt_tmp_copy(&k1en5, &k1e5);
    flt_tmp_invert(&k1en5);
    flt_tmp_copy(&k1en8, &k1e8);
    flt_tmp_invert(&k1en8);
    /* One half */
    flt_tmp_copy(&k5en1, &k2);
    flt_tmp_invert(&k5en1);
    /* Done */
    _prepared = true;
  }
}
