/*
    Contents: "testing" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#include "flt.h"

#define TEST(F)	if (!test_flt_##F()) fprintf(stderr, "fail: %s\n", #F)

static bool test_flt_ldexp();		static bool test_flt_frexp();			static bool test_flt_modf();			static bool test_flt_fmod();
static bool test_flt_fabs();		static bool test_flt_ceil();			static bool test_flt_floor();			static bool test_flt_round();
static bool test_flt_trunc();		static bool test_flt_sqrt();			static bool test_flt_log();				static bool test_flt_log10();
static bool test_flt_exp();			static bool test_flt_exp10();			static bool test_flt_pow();				static bool test_flt_sin();
static bool test_flt_cos();			static bool test_flt_tan();				static bool test_flt_asin();			static bool test_flt_acos();
static bool test_flt_atan();		static bool test_flt_atan2();			static bool test_flt_sinh();			static bool test_flt_cosh();
static bool test_flt_tanh();		static bool test_flt_asinh();			static bool test_flt_acosh();			static bool test_flt_atanh();
static bool test_flt_classify();	static bool test_flt_isinf();			static bool test_flt_isnan();			static bool test_flt_isnormal();
static bool test_flt_issubnormal();	static bool test_flt_iszero();			static bool test_flt_isfinite();		static bool test_flt_compare();
static bool test_flt_add();			static bool test_flt_subtract();		static bool test_flt_multiply();		static bool test_flt_divide();
static bool test_flt_add_into();	static bool test_flt_subtract_into();	static bool test_flt_multiply_into();	static bool test_flt_divide_into();
static bool test_flt_negated();		static bool test_flt_inverted();		static bool test_flt_atof();			static bool test_flt_ftoa();
static bool test_flt_ltof();		static bool test_flt_ultof();			static bool test_flt_ftol();			static bool test_flt_ftoul();
static bool test_flt_fmin();		static bool test_flt_fmax();			static bool test_flt_fsgn();			static bool test_flt_hypot();
static bool test_flt_log2();		static bool test_flt_exp2();

int main() {
	TEST(ldexp);		TEST(frexp);			TEST(modf);				TEST(fmod);
	TEST(fabs);			TEST(ceil);				TEST(floor);			TEST(round);
	TEST(trunc); 		TEST(sqrt);				TEST(log);				TEST(log10);
	TEST(exp);			TEST(exp10);			TEST(pow);				TEST(sin);
	TEST(cos);			TEST(tan);				TEST(asin);				TEST(acos);
	TEST(atan);			TEST(atan2);			TEST(sinh);				TEST(cosh);
	TEST(tanh);			TEST(asinh);			TEST(acosh);			TEST(atanh);
	TEST(classify);		TEST(isinf);			TEST(isnan);			TEST(isnormal);
	TEST(issubnormal);	TEST(iszero);			TEST(isfinite);			TEST(compare);
	TEST(add);			TEST(subtract);			TEST(multiply);			TEST(divide);
	TEST(add_into);		TEST(subtract_into);	TEST(multiply_into);	TEST(divide_into);
	TEST(negated);		TEST(inverted);			TEST(atof);				TEST(ftoa);
	TEST(ltof);			TEST(ultof);			TEST(ftol);				TEST(ftoul);
	TEST(fmin);			TEST(fmax);				TEST(fsgn);				TEST(hypot);
	TEST(log2);			TEST(exp2);
	return EXIT_SUCCESS;
}

FLT values[] = {
	FLT_POS_INF, FLT_NEG_INF, FLT_NAN, FLT_POS_0, FLT_NEG_0,
	FLT_POS_1, FLT_NEG_1, FLT_POS_PI_2, FLT_NEG_PI_2,
	/* +subnormal, -subnormal, +large, -large */
	0x0033336C, 0x8033336C, 0x7E0D7F54, 0xFE0D7F54,
	/* regular numbers: 1.6, -1.6, 0.7, -0.7 */
	0x3FCCCCCD, 0xBFCCCCCD, 0x3F333333, 0xBF333333
};

unsigned short crc16(unsigned short crc, char *s) {
	static const unsigned short CRC16[] = {
    	0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50A5,  0x60C6,  0x70E7,
    	0x8108,  0x9129,  0xA14A,  0xB16B,  0xC18C,  0xD1AD,  0xE1CE,  0xF1EF,
    	0x1231,  0x0210,  0x3273,  0x2252,  0x52B5,  0x4294,  0x72F7,  0x62D6,
    	0x9339,  0x8318,  0xB37B,  0xA35A,  0xD3BD,  0xC39C,  0xF3FF,  0xE3DE,
    	0x2462,  0x3443,  0x0420,  0x1401,  0x64E6,  0x74C7,  0x44A4,  0x5485,
    	0xA56A,  0xB54B,  0x8528,  0x9509,  0xE5EE,  0xF5CF,  0xC5AC,  0xD58D,
    	0x3653,  0x2672,  0x1611,  0x0630,  0x76D7,  0x66F6,  0x5695,  0x46B4,
    	0xB75B,  0xA77A,  0x9719,  0x8738,  0xF7DF,  0xE7FE,  0xD79D,  0xC7BC,
    	0x48C4,  0x58E5,  0x6886,  0x78A7,  0x0840,  0x1861,  0x2802,  0x3823,
    	0xC9CC,  0xD9ED,  0xE98E,  0xF9AF,  0x8948,  0x9969,  0xA90A,  0xB92B,
    	0x5AF5,  0x4AD4,  0x7AB7,  0x6A96,  0x1A71,  0x0A50,  0x3A33,  0x2A12,
    	0xDBFD,  0xCBDC,  0xFBBF,  0xEB9E,  0x9B79,  0x8B58,  0xBB3B,  0xAB1A,
    	0x6CA6,  0x7C87,  0x4CE4,  0x5CC5,  0x2C22,  0x3C03,  0x0C60,  0x1C41,
    	0xEDAE,  0xFD8F,  0xCDEC,  0xDDCD,  0xAD2A,  0xBD0B,  0x8D68,  0x9D49,
    	0x7E97,  0x6EB6,  0x5ED5,  0x4EF4,  0x3E13,  0x2E32,  0x1E51,  0x0E70,
    	0xFF9F,  0xEFBE,  0xDFDD,  0xCFFC,  0xBF1B,  0xAF3A,  0x9F59,  0x8F78,
    	0x9188,  0x81A9,  0xB1CA,  0xA1EB,  0xD10C,  0xC12D,  0xF14E,  0xE16F,
    	0x1080,  0x00A1,  0x30C2,  0x20E3,  0x5004,  0x4025,  0x7046,  0x6067,
    	0x83B9,  0x9398,  0xA3FB,  0xB3DA,  0xC33D,  0xD31C,  0xE37F,  0xF35E,
    	0x02B1,  0x1290,  0x22F3,  0x32D2,  0x4235,  0x5214,  0x6277,  0x7256,
    	0xB5EA,  0xA5CB,  0x95A8,  0x8589,  0xF56E,  0xE54F,  0xD52C,  0xC50D,
    	0x34E2,  0x24C3,  0x14A0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    	0xA7DB,  0xB7FA,  0x8799,  0x97B8,  0xE75F,  0xF77E,  0xC71D,  0xD73C,
    	0x26D3,  0x36F2,  0x0691,  0x16B0,  0x6657,  0x7676,  0x4615,  0x5634,
    	0xD94C,  0xC96D,  0xF90E,  0xE92F,  0x99C8,  0x89E9,  0xB98A,  0xA9AB,
    	0x5844,  0x4865,  0x7806,  0x6827,  0x18C0,  0x08E1,  0x3882,  0x28A3,
    	0xCB7D,  0xDB5C,  0xEB3F,  0xFB1E,  0x8BF9,  0x9BD8,  0xABBB,  0xBB9A,
    	0x4A75,  0x5A54,  0x6A37,  0x7A16,  0x0AF1,  0x1AD0,  0x2AB3,  0x3A92,
    	0xFD2E,  0xED0F,  0xDD6C,  0xCD4D,  0xBDAA,  0xAD8B,  0x9DE8,  0x8DC9,
    	0x7C26,  0x6C07,  0x5C64,  0x4C45,  0x3CA2,  0x2C83,  0x1CE0,  0x0CC1,
    	0xEF1F,  0xFF3E,  0xCF5D,  0xDF7C,  0xAF9B,  0xBFBA,  0x8FD9,  0x9FF8,
    	0x6E17,  0x7E36,  0x4E55,  0x5E74,  0x2E93,  0x3EB2,  0x0ED1,  0x1EF0
	};
	int i;
	if (s) {
		for (i = 0; s[i]; ++i)
			crc = (CRC16[((crc) >> CHAR_BIT) ^ (unsigned char)(s[i])] ^ (crc) << CHAR_BIT);
	}
	return crc;
}

static bool test_flt_ldexp() {
	char s[100];
	unsigned short crc;
	int i, j, exponents[] = {-200, -100, -50, -20, -10, -5, -2, -1, 0, 1, 2, 5, 10, 20, 50, 100, 200};
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		for (j = 0; j < sizeof(exponents)/sizeof(int); ++j) {
			snprintf(s, sizeof(s), "ldexp(%s, %d) = %s", flt_ftoa(values[i], "%e"), exponents[j], flt_ftoa(flt_ldexp(values[i], exponents[j]), "%e"));
			crc = crc16(crc, s);
			printf("ldexp %ld %s\n", strlen(s), s);
		}
	}
	printf("ldexp 0x%04X\n", crc);
	return crc == 0x2504;
}

static bool test_flt_frexp() {
	char s[100];
	unsigned short crc;
	int i;
	FLT f;
	int exponent;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		f = flt_frexp(values[i], &exponent);
		snprintf(s, sizeof(s), "frexp(%s) = %s * 2^%d", flt_ftoa(values[i], "%e"), flt_ftoa(f, "%e"), exponent);
		crc = crc16(crc, s);
		printf("frexp %ld %s\n", strlen(s), s);
	}
	printf("frexp 0x%04X\n", crc);
	return crc == 0x8F02;
}

static bool test_flt_modf() {
	char s[100];
	unsigned short crc;
	int i;
	FLT f, g;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		f = flt_modf(values[i], &g);
		snprintf(s, sizeof(s), "modf(%s) = %s + %s", flt_ftoa(values[i], "%e"), flt_ftoa(g, "%e"), flt_ftoa(f, "%e"));
		crc = crc16(crc, s);
		printf("modf %ld %s\n", strlen(s), s);
	}
	printf("modf 0x%04X\n", crc);
	return crc == 0x7EF7;
}

static bool test_flt_fmod() {
	char s[100];
	unsigned short crc;
	int i, j;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		for (j = 0; j < sizeof(values)/sizeof(FLT); ++j) {
			snprintf(s, sizeof(s), "fmod(%s, %s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(values[j], "%e"), flt_ftoa(flt_fmod(values[i], values[j]), "%e"));
			crc = crc16(crc, s);
			printf("fmod %ld %s\n", strlen(s), s);
		}
	}
	printf("fmod 0x%04X\n", crc);
	return crc == 0x26AC;
}

static bool test_flt_fabs() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "fabs(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_fabs(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("fabs %ld %s\n", strlen(s), s);
	}
	printf("fabs 0x%04X\n", crc);
	return crc == 0x5744;
}

static bool test_flt_ceil() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "ceil(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_ceil(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("ceil %ld %s\n", strlen(s), s);
	}
	printf("ceil 0x%04X\n", crc);
	return crc == 0x5134;
}

static bool test_flt_floor() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "floor(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_floor(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("floor %ld %s\n", strlen(s), s);
	}
	printf("floor 0x%04X\n", crc);
	return crc == 0x7091;
}

static bool test_flt_round() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "round(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_round(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("round %ld %s\n", strlen(s), s);
	}
	printf("round 0x%04X\n", crc);
	return crc == 0x1C72;
}

static bool test_flt_trunc() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "trunc(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_trunc(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("trunc %ld %s\n", strlen(s), s);
	}
	printf("trunc 0x%04X\n", crc);
	return crc == 0x3E9E;
}

static bool test_flt_sqrt() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "sqrt(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_sqrt(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("sqrt %ld %s\n", strlen(s), s);
	}
	printf("sqrt 0x%04X\n", crc);
	return crc == 0xF12B;
}

static bool test_flt_log() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "log(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_log(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("log %ld %s\n", strlen(s), s);
	}
	printf("log 0x%04X\n", crc);
	return crc == 0x4A19;
}

static bool test_flt_log10() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "log10(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_log10(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("log10 %ld %s\n", strlen(s), s);
	}
	printf("log10 0x%04X\n", crc);
	return crc == 0xD046;
}

static bool test_flt_exp() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "exp(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_exp(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("exp %ld %s\n", strlen(s), s);
	}
	printf("exp 0x%04X\n", crc);
	return crc == 0x6056;
}

static bool test_flt_exp10() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "exp10(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_exp10(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("exp10 %ld %s\n", strlen(s), s);
	}
	printf("exp10 0x%04X\n", crc);
	return crc == 0xCC58;
}

static bool test_flt_pow() {
	char s[100];
	unsigned short crc;
	int i, j;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		for (j = 0; j < sizeof(values)/sizeof(FLT); ++j) {
			snprintf(s, sizeof(s), "pow(%s, %s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(values[j], "%e"), flt_ftoa(flt_pow(values[i], values[j]), "%e"));
			crc = crc16(crc, s);
			printf("pow %ld %s\n", strlen(s), s);
		}
	}
	printf("pow 0x%04X\n", crc);
	return crc == 0x84CB;
}

static bool test_flt_sin() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "sin(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_sin(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("sin %ld %s\n", strlen(s), s);
	}
	printf("sin 0x%04X\n", crc);
	return crc == 0xBBF6;
}

static bool test_flt_cos() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "cos(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_cos(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("cos %ld %s\n", strlen(s), s);
	}
	printf("cos 0x%04X\n", crc);
	return crc == 0x7225;
}

static bool test_flt_tan() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "tan(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_tan(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("tan %ld %s\n", strlen(s), s);
	}
	printf("tan 0x%04X\n", crc);
	return crc == 0xA710;
}

static bool test_flt_asin() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "asin(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_asin(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("asin %ld %s\n", strlen(s), s);
	}
	printf("asin 0x%04X\n", crc);
	return crc == 0x5EB8;
}

static bool test_flt_acos() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "acos(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_acos(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("acos %ld %s\n", strlen(s), s);
	}
	printf("acos 0x%04X\n", crc);
	return crc == 0xE5BB;
}

static bool test_flt_atan() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "atan(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_atan(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("atan %ld %s\n", strlen(s), s);
	}
	printf("atan 0x%04X\n", crc);
	return crc == 0xD50F;
}

static bool test_flt_atan2() {
	char s[100];
	unsigned short crc;
	int i, j;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		for (j = 0; j < sizeof(values)/sizeof(FLT); ++j) {
			snprintf(s, sizeof(s), "atan2(%s, %s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(values[j], "%e"), flt_ftoa(flt_atan2(values[i], values[j]), "%e"));
			crc = crc16(crc, s);
			printf("atan2 %ld %s\n", strlen(s), s);
		}
	}
	printf("atan2 0x%04X\n", crc);
	return crc == 0x8AE4;
}

static bool test_flt_sinh() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "sinh(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_sinh(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("sinh %ld %s\n", strlen(s), s);
	}
	printf("sinh 0x%04X\n", crc);
	return crc == 0x1A4B;
}

static bool test_flt_cosh() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "cosh(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_cosh(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("cosh %ld %s\n", strlen(s), s);
	}
	printf("cosh 0x%04X\n", crc);
	return crc == 0xD0FB;
}

static bool test_flt_tanh() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "tanh(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_tanh(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("tanh %ld %s\n", strlen(s), s);
	}
	printf("tanh 0x%04X\n", crc);
	return crc == 0x2E62;
}

static bool test_flt_asinh() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "asinh(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_asinh(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("asinh %ld %s\n", strlen(s), s);
	}
	printf("asinh 0x%04X\n", crc);
	return crc == 0x5201;
}

static bool test_flt_acosh() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "acosh(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_acosh(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("acosh %ld %s\n", strlen(s), s);
	}
	printf("acosh 0x%04X\n", crc);
	return crc == 0xA918;
}

static bool test_flt_atanh() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "atanh(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_atanh(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("atanh %ld %s\n", strlen(s), s);
	}
	printf("atanh 0x%04X\n", crc);
	return crc == 0x1943;
}

static bool test_flt_classify() {
	char s[50], *classes[] = {"E_INFINITE", "E_NAN", "E_NORMAL", "E_SUBNORMAL", "E_ZERO"};
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "classify(%s) = %s", flt_ftoa(values[i], "%e"), classes[flt_classify(values[i]) - 1]);
		crc = crc16(crc, s);
		printf("classify %ld %s\n", strlen(s), s);
	}
	printf("classify 0x%04X\n", crc);
	return crc == 0x11A3;
}

static bool test_flt_isinf() {
	char s[50], *true_or_false[] = {"false", "true"};
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "isinf(%s) = %s", flt_ftoa(values[i], "%e"), true_or_false[flt_isinf(values[i])]);
		crc = crc16(crc, s);
		printf("isinf %ld %s\n", strlen(s), s);
	}
	printf("isinf 0x%04X\n", crc);
	return crc == 0x3B1D;
}

static bool test_flt_isnan() {
	char s[50], *true_or_false[] = {"false", "true"};
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "isnan(%s) = %s", flt_ftoa(values[i], "%e"), true_or_false[flt_isnan(values[i])]);
		crc = crc16(crc, s);
		printf("isnan %ld %s\n", strlen(s), s);
	}
	printf("isnan 0x%04X\n", crc);
	return crc == 0x3C5F;
}

static bool test_flt_isnormal() {
	char s[50], *true_or_false[] = {"false", "true"};
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "isnormal(%s) = %s", flt_ftoa(values[i], "%e"), true_or_false[flt_isnormal(values[i])]);
		crc = crc16(crc, s);
		printf("isnormal %ld %s\n", strlen(s), s);
	}
	printf("isnormal 0x%04X\n", crc);
	return crc == 0x8E4C;
}

static bool test_flt_issubnormal() {
	char s[50], *true_or_false[] = {"false", "true"};
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "issubnormal(%s) = %s", flt_ftoa(values[i], "%e"), true_or_false[flt_issubnormal(values[i])]);
		crc = crc16(crc, s);
		printf("issubnormal %ld %s\n", strlen(s), s);
	}
	printf("issubnormal 0x%04X\n", crc);
	return crc == 0x10FE;
}

static bool test_flt_iszero() {
	char s[50], *true_or_false[] = {"false", "true"};
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "iszero(%s) = %s", flt_ftoa(values[i], "%e"), true_or_false[flt_iszero(values[i])]);
		crc = crc16(crc, s);
		printf("iszero %ld %s\n", strlen(s), s);
	}
	printf("iszero 0x%04X\n", crc);
	return crc == 0xED25;
}

static bool test_flt_isfinite() {
	char s[50], *true_or_false[] = {"false", "true"};
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "isfinite(%s) = %s", flt_ftoa(values[i], "%e"), true_or_false[flt_isfinite(values[i])]);
		crc = crc16(crc, s);
		printf("isfinite %ld %s\n", strlen(s), s);
	}
	printf("isfinite 0x%04X\n", crc);
	return crc == 0x94B1;
}

static bool test_flt_compare() {
	char s[100], *symbols[] = {"==", "<", ">", "<=", ">=", "!="}, *true_or_false[] = {"false", "true"};
	unsigned short crc;
	int i, j, k;
	E_COMPARE relations[] = {E_EQUAL_TO, E_LESS_THAN, E_GREATER_THAN, E_LESS_THAN_OR_EQUAL_TO, E_GREATER_THAN_OR_EQUAL_TO, E_NOT_EQUAL_TO};
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		for (j = 0; j < sizeof(values)/sizeof(FLT); ++j) {
			for (k = 0; k < sizeof(relations)/sizeof(E_COMPARE); ++k) {
				snprintf(s, sizeof(s), "compare(%s, %s, %s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(values[j], "%e"), symbols[k],
					true_or_false[flt_compare(values[i], values[j], relations[k])]);
				crc = crc16(crc, s);
				printf("compare %ld %s\n", strlen(s), s);
			}
		}
	}
	printf("compare 0x%04X\n", crc);
	return crc == 0xE71B;
}

static bool test_flt_add() {
	char s[100];
	unsigned short crc;
	int i, j;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		for (j = 0; j < sizeof(values)/sizeof(FLT); ++j) {
			snprintf(s, sizeof(s), "add(%s, %s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(values[j], "%e"), flt_ftoa(flt_add(values[i], values[j]), "%e"));
			crc = crc16(crc, s);
			printf("add %ld %s\n", strlen(s), s);
		}
	}
	printf("add 0x%04X\n", crc);
	return crc == 0x0AF6;
}

static bool test_flt_subtract() {
	char s[100];
	unsigned short crc;
	int i, j;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		for (j = 0; j < sizeof(values)/sizeof(FLT); ++j) {
			snprintf(s, sizeof(s), "subtract(%s, %s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(values[j], "%e"), flt_ftoa(flt_subtract(values[i], values[j]), "%e"));
			crc = crc16(crc, s);
			printf("subtract %ld %s\n", strlen(s), s);
		}
	}
	printf("subtract 0x%04X\n", crc);
	return crc == 0x0B77;
}

static bool test_flt_multiply() {
	char s[100];
	unsigned short crc;
	int i, j;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		for (j = 0; j < sizeof(values)/sizeof(FLT); ++j) {
			snprintf(s, sizeof(s), "multiply(%s, %s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(values[j], "%e"), flt_ftoa(flt_multiply(values[i], values[j]), "%e"));
			crc = crc16(crc, s);
			printf("multiply %ld %s\n", strlen(s), s);
		}
	}
	printf("multiply 0x%04X\n", crc);
	return crc == 0xC717;
}

static bool test_flt_divide() {
	char s[100];
	unsigned short crc;
	int i, j;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		for (j = 0; j < sizeof(values)/sizeof(FLT); ++j) {
			snprintf(s, sizeof(s), "divide(%s, %s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(values[j], "%e"), flt_ftoa(flt_divide(values[i], values[j]), "%e"));
			crc = crc16(crc, s);
			printf("divide %ld %s\n", strlen(s), s);
		}
	}
	printf("divide 0x%04X\n", crc);
	return crc == 0x6B83;
}

static bool test_flt_add_into() {
	char s[100];
	unsigned short crc;
	int i, j;
	FLT f;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		for (j = 0; j < sizeof(values)/sizeof(FLT); ++j) {
			f = values[i];
			snprintf(s, sizeof(s), "add_into(%s, %s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(values[j], "%e"), flt_ftoa(flt_add_into(&f, values[j]), "%e"));
			crc = crc16(crc, s);
			printf("add_into %ld %s\n", strlen(s), s);
		}
	}
	printf("add_into 0x%04X\n", crc);
	return crc == 0x4FF3;
}

static bool test_flt_subtract_into() {
	char s[100];
	unsigned short crc;
	int i, j;
	FLT f;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		for (j = 0; j < sizeof(values)/sizeof(FLT); ++j) {
			f = values[i];
			snprintf(s, sizeof(s), "subtract_into(%s, %s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(values[j], "%e"), flt_ftoa(flt_subtract_into(&f, values[j]), "%e"));
			crc = crc16(crc, s);
			printf("subtract_into %ld %s\n", strlen(s), s);
		}
	}
	printf("subtract_into 0x%04X\n", crc);
	return crc == 0xEAC8;
}

static bool test_flt_multiply_into() {
	char s[100];
	unsigned short crc;
	int i, j;
	FLT f;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		for (j = 0; j < sizeof(values)/sizeof(FLT); ++j) {
			f = values[i];
			snprintf(s, sizeof(s), "multiply_into(%s, %s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(values[j], "%e"), flt_ftoa(flt_multiply_into(&f, values[j]), "%e"));
			crc = crc16(crc, s);
			printf("multiply_into %ld %s\n", strlen(s), s);
		}
	}
	printf("multiply_into 0x%04X\n", crc);
	return crc == 0x158A;
}

static bool test_flt_divide_into() {
	char s[100];
	unsigned short crc;
	int i, j;
	FLT f;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		for (j = 0; j < sizeof(values)/sizeof(FLT); ++j) {
			f = values[i];
			snprintf(s, sizeof(s), "divide_into(%s, %s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(values[j], "%e"), flt_ftoa(flt_divide_into(&f, values[j]), "%e"));
			crc = crc16(crc, s);
			printf("divide_into %ld %s\n", strlen(s), s);
		}
	}
	printf("divide_into 0x%04X\n", crc);
	return crc == 0xF16A;
}

static bool test_flt_negated() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "negated(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_negated(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("negated %ld %s\n", strlen(s), s);
	}
	printf("negated 0x%04X\n", crc);
	return crc == 0x25BF;
}

static bool test_flt_inverted() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "inverted(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_inverted(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("inverted %ld %s\n", strlen(s), s);
	}
	printf("inverted 0x%04X\n", crc);
	return crc == 0x48D3;
}

static bool test_flt_atof() {
	char s[100], *formats[] = {"%e", "%f", "%g", "%.0e", "%.0f", "%.0g", "%.1e", "%.1f", "%.1g", "%.9e", "%.9f", "%.9g", "%+E", "%+F", "%+G"};
	unsigned short crc;
	int i, j;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		for (j = 0; j < sizeof(formats)/sizeof(char *); ++j) {
			snprintf(s, sizeof(s), "atof(\"%s\") = %s", flt_ftoa(values[i], formats[j]), flt_ftoa(flt_atof(flt_ftoa(values[i], formats[j])), "%e"));
			crc = crc16(crc, s);
			printf("atof %ld %s\n", strlen(s), s);
		}
	}
	printf("atof 0x%04X\n", crc);
	return crc == 0x18F3;
}

static bool test_flt_ftoa() {
	char s[100], *formats[] = {"%e", "%f", "%g", "%.0e", "%.0f", "%.0g", "%.1e", "%.1f", "%.1g", "%.9e", "%.9f", "%.9g", "%+E", "%+F", "%+G"};
	unsigned short crc;
	int i, j;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		for (j = 0; j < sizeof(formats)/sizeof(char *); ++j) {
			snprintf(s, sizeof(s), "ftoa(\"%s\") = %s", formats[j], flt_ftoa(values[i], formats[j]));
			crc = crc16(crc, s);
			printf("ftoa %ld %s\n", strlen(s), s);
		}
	}
	printf("ftoa 0x%04X\n", crc);
	return crc == 0x0FEF;
}

static bool test_flt_ltof() {
	char s[50];
	unsigned short crc;
	int32_t cases[] = {-2147483647, -65536, -65535, -1, 0, 1, 65536, 65536, 2147483647};
	int i;
	for (i = 0, crc = 0; i < sizeof(cases)/sizeof(int32_t); ++i) {
		snprintf(s, sizeof(s), "ltof(%ld) = %s", (long)cases[i], flt_ftoa(flt_ltof(cases[i]), "%e"));
		crc = crc16(crc, s);
		printf("ltof %ld %s\n", strlen(s), s);
	}
	printf("ltof 0x%04X\n", crc);
	return crc == 0xFD75;
}

static bool test_flt_ultof() {
	char s[50];
	unsigned short crc;
	uint32_t cases[] = {0, 1, 65536, 65536, 2147483647, 2147483648, 4294967295};
	int i;
	for (i = 0, crc = 0; i < sizeof(cases)/sizeof(uint32_t); ++i) {
		snprintf(s, sizeof(s), "ultof(%lu) = %s", (unsigned long)cases[i], flt_ftoa(flt_ultof(cases[i]), "%e"));
		crc = crc16(crc, s);
		printf("ultof %ld %s\n", strlen(s), s);
	}
	printf("ultof 0x%04X\n", crc);
	return crc == 0x8B81;
}

static bool test_flt_ftol() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "ftol(%s) = %d", flt_ftoa(values[i], "%e"), flt_ftol(values[i]));
		crc = crc16(crc, s);
		printf("ftol %ld %s\n", strlen(s), s);
	}
	printf("ftol 0x%04X\n", crc);
	return crc == 0xDFBE;
}

static bool test_flt_ftoul() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "ftoul(%s) = %u", flt_ftoa(values[i], "%e"), flt_ftoul(values[i]));
		crc = crc16(crc, s);
		printf("ftoul %ld %s\n", strlen(s), s);
	}
	printf("ftoul 0x%04X\n", crc);
	return crc == 0x0F34;
}

static bool test_flt_fmin() {
	char s[100];
	unsigned short crc;
	int i, j;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		for (j = 0; j < sizeof(values)/sizeof(FLT); ++j) {
			snprintf(s, sizeof(s), "fmin(%s, %s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(values[j], "%e"), flt_ftoa(flt_fmin(values[i], values[j]), "%e"));
			crc = crc16(crc, s);
			printf("fmin %ld %s\n", strlen(s), s);
		}
	}
	printf("fmin 0x%04X\n", crc);
	return crc == 0x7D13;
}

static bool test_flt_fmax() {
	char s[100];
	unsigned short crc;
	int i, j;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		for (j = 0; j < sizeof(values)/sizeof(FLT); ++j) {
			snprintf(s, sizeof(s), "fmax(%s, %s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(values[j], "%e"), flt_ftoa(flt_fmax(values[i], values[j]), "%e"));
			crc = crc16(crc, s);
			printf("fmax %ld %s\n", strlen(s), s);
		}
	}
	printf("fmax 0x%04X\n", crc);
	return crc == 0xC53D;
}

static bool test_flt_fsgn() {
	char s[50], *signs[] = {"-1", "0", "+1"};
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "fsgn(%s) = %s", flt_ftoa(values[i], "%e"), signs[1 + flt_fsgn(values[i])]);
		crc = crc16(crc, s);
		printf("fsgn %ld %s\n", strlen(s), s);
	}
	printf("fsgn 0x%04X\n", crc);
	return crc == 0xB41B;
}

static bool test_flt_hypot() {
	char s[100];
	unsigned short crc;
	int i, j;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		for (j = 0; j < sizeof(values)/sizeof(FLT); ++j) {
			snprintf(s, sizeof(s), "hypot(%s, %s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(values[j], "%e"), flt_ftoa(flt_hypot(values[i], values[j]), "%e"));
			crc = crc16(crc, s);
			printf("hypot %ld %s\n", strlen(s), s);
		}
	}
	printf("hypot 0x%04X\n", crc);
	return crc == 0xD7E4;
}

static bool test_flt_log2() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "log2(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_log2(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("log2 %ld %s\n", strlen(s), s);
	}
	printf("log2 0x%04X\n", crc);
	return crc == 0xCC8D;
}

static bool test_flt_exp2() {
	char s[50];
	unsigned short crc;
	int i;
	for (i = 0, crc = 0; i < sizeof(values)/sizeof(FLT); ++i) {
		snprintf(s, sizeof(s), "exp2(%s) = %s", flt_ftoa(values[i], "%e"), flt_ftoa(flt_exp2(values[i]), "%e"));
		crc = crc16(crc, s);
		printf("exp2 %ld %s\n", strlen(s), s);
	}
	printf("exp2 0x%04X\n", crc);
	return crc == 0x9124;
}
