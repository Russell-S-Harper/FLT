#include <stdio.h>

int main() {
	char buffer[250];
	int count;
	float e, f, g, h, i;

	for (;;) {
		printf("Enter up to five numbers (ctrl-C to exit): ");
		fgets(buffer, sizeof(buffer), stdin);
		count = sscanf(buffer, "%f %f %f %f %f", &e, &f, &g, &h, &i);
		switch (count) {
			case 5:
				printf("Average (%d): %.3g\n", count, (e + f + g + h + i) / 5.0f);
				break;
			case 4:
				printf("Average (%d): %.3g\n", count, (e + f + g + h) / 4.0f);
				break;
			case 3:
				printf("Average (%d): %.3g\n", count, (e + f + g) / 3.0f);
				break;
			case 2:
				printf("Average (%d): %.3g\n", count, (e + f) / 2.0f);
				break;
			case 1:
				printf("Average (%d): %.3g\n", count, e);
				break;
			default:
				printf("Average (%d): %.3g\n", count, e);
				break;
		}
	}
	return 0;
}
