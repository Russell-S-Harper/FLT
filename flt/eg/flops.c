/*
    Contents: "flops" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: russell.s.harper@gmail.com
*/

#include <stdio.h>
#include <math.h>
#include <time.h>

/* <time.h> may not provide CLOCKS_PER_SEC for __CX16__, so hard coding as a fallback! */
#if  defined (__CX16__) && !defined (CLOCKS_PER_SEC)
#define CLOCKS_PER_SEC	60
#endif

int main() {
	int i;
	double a, b, d;
	clock_t t;

	printf("           Error\n");
	printf("     FLOPS (ppb)\n");
	for (i = 0, a = 10.0, b = 0.01, t = -clock(); i < 1000; ++i)
		a += b;
	t += clock();
	d = (double)t / (double)CLOCKS_PER_SEC;
	printf("FADD %5.1F %4.1F\n", 1000.0 / d, 1000000.0 * fabs(1.0 - a / 20.0));

	for (i = 0, a = 20.0, b = 0.01, t = -clock(); i < 1000; ++i)
		a -= b;
	t += clock();
	d = (double)t / (double)CLOCKS_PER_SEC;
	printf("FSUB %5.1F %4.1F\n", 1000.0 / d, 1000000.0 * fabs(1.0 - a / 10.0));

	for (i = 0, a = 10.0, b = 1.001, t = -clock(); i < 1000; ++i)
		a *= b;
	t += clock();
	d = (double)t / (double)CLOCKS_PER_SEC;
	printf("FMUL %5.1F %4.1F\n", 1000.0 / d, 1000000.0 * fabs(1.0 - a / 27.1692393223559));

	for (i = 0, a = 10.0, b = 1.001, t = -clock(); i < 1000; ++i)
		a /= b;
	t += clock();
	d = (double)t / (double)CLOCKS_PER_SEC;
	printf("FDIV %5.1F %4.1F\n", 1000.0 / d, 1000000.0 * fabs(1.0 - a / 3.68063304288818));
	
	return 0;
}
