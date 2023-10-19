/*
    Contents: "averages" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: flt@russell-harper.com
*/
#include <stdio.h>

int main() {
	char buffer[250];
	int i, j, count;
	float F[5], total;

	for (;;) {
		printf("Enter up to five numbers (Ctrl-C to exit): ");
		fgets(buffer, sizeof(buffer), stdin);
		j = sscanf(buffer, "%f %f %f %f %f", F + 0, F + 1, F + 2, F + 3, F + 4);
		for (i = count = 0, total = 0.0; i < j; ++i) {
			if (!isnan(F[i])) {
				total += F[i];
				++count;
			}
		}
		if (count)
			printf("Average (%d): %.3g\n", count, total / count);
		else
			printf("No numbers entered!\n");
	}
	return 0;
}
