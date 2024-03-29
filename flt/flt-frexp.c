/*
    Contents: "frexp" C source code (c) 2023
  Repository: https://github.com/Russell-S-Harper/FLT
     Contact: russell.s.harper@gmail.com
*/
#include <stdlib.h>

#include "flt-tmp.h"

FLT flt_frexp(const FLT f, int *exponent) {
	FLT result;
	flt_tmp t;
	/* Destination can't be NULL */
	if (exponent == NULL)
		exit(EXIT_FAILURE);
	flt_to_tmp(&f, &t);
	*exponent = t.e;
	t.e = 0;
	tmp_to_flt(&t, &result);
	return result;
}
