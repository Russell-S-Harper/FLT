/*
    Contents: "test" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: russell.s.harper@gmail.com
*/

/*
  To Compile: gcc -o errors errors.c ../*.c -I .. -lm
      To Run: ./errors
*/

#include <stdio.h>
#include <math.h>
#include <string.h>

#include "flt.h"

double accuracy(double min, double max) {
	return -log10(fmax(fabs(min), fabs(max)));
}

int main() {
	float a, b;
	double t, min, max;
	FLT f, g;
 
	fputs("Func\tDigits\n", stdout);

	for (a = 0.0f, min = FLT_MAX, max = FLT_MIN; a < 6.28318530718f; a += 6.28318530718f/131071.0f) {
		memcpy(&f, &a, sizeof(float));
		g = flt_sin(f);
		memcpy(&b, &g, sizeof(float));
		t = sin(a) - b;
		if (min > t)
			min = t;
		if (max < t)
			max = t;
	}
	printf("sin\t%.2f\n", accuracy(min, max));

	for (a = 0.0f, min = FLT_MAX, max = FLT_MIN; a < 6.28318530718f; a += 6.28318530718f/131071.0f) {
		memcpy(&f, &a, sizeof(float));
		g = flt_cos(f);
		memcpy(&b, &g, sizeof(float));
		t = cos(a) - b;
		if (min > t)
			min = t;
		if (max < t)
			max = t;
	}
	printf("cos\t%.2f\n", accuracy(min, max));

	for (a = 0.0f, min = FLT_MAX, max = FLT_MIN; a < 1.0f; a += 1.0f/131071.0f) {
		memcpy(&f, &a, sizeof(float));
		g = flt_atan(f);
		memcpy(&b, &g, sizeof(float));
		t = atan(a) - b;
		if (min > t)
			min = t;
		if (max < t)
			max = t;
	}
	printf("atan\t%.2f\n", accuracy(min, max));

	for (a = 1.0f, min = FLT_MAX, max = FLT_MIN; a < 2.0f; a += 1.0f/131071.0f) {
		memcpy(&f, &a, sizeof(float));
		g = flt_exp2(f);
		memcpy(&b, &g, sizeof(float));
		t = exp2(a) - b;
		if (min > t)
			min = t;
		if (max < t)
			max = t;
	}
	printf("exp2\t%.2f\n", accuracy(min, max));

	for (a = 1.0f, min = FLT_MAX, max = FLT_MIN; a < 2.0f; a += 1.0f/131071.0f) {
		memcpy(&f, &a, sizeof(float));
		g = flt_log2(f);
		memcpy(&b, &g, sizeof(float));
		t = log2(a) - b;
		if (min > t)
			min = t;
		if (max < t)
			max = t;
	}
	printf("log2\t%.2f\n", accuracy(min, max));

	for (a = 1.0f, min = FLT_MAX, max = FLT_MIN; a < 2.0f; a += 1.0f/131071.0f) {
		memcpy(&f, &a, sizeof(float));
		g = flt_inverted(f);
		memcpy(&b, &g, sizeof(float));
		t = 1.0/a - b;
		if (min > t)
			min = t;
		if (max < t)
			max = t;
	}
	printf("invert\t%.2f\n", accuracy(min, max));

	for (a = -6.28318530718f, min = FLT_MAX, max = FLT_MIN; a < +6.28318530718f; a += 6.28318530718f/131071.0f) {
		memcpy(&f, &a, sizeof(float));
		g = flt_hypot(flt_sin(f), flt_cos(f));
		memcpy(&b, &g, sizeof(float));
		t = 1.0 - b;
		if (min > t)
			min = t;
		if (max < t)
			max = t;
	}
	printf("hypot\t%.2f\n", accuracy(min, max));

	return 0;
}
