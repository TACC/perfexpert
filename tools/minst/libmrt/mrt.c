
#include <stdio.h>
#include <stdlib.h>

#include "mrt.h"

inline void indigo__record_c(int read_write, int line_number, void* addr, int var_idx)
{
	fprintf (stderr, "r/w: %d, line_number: %ld, addr: %p, var_idx: %ld\n", read_write, line_number, addr, var_idx);
	exit(1);
}

inline void indigo__record_f_(int *read_write, int *line_number, void* addr, int *var_idx)
{
	fprintf (stderr, "r/w: %d, line_number: %ld, addr: %p, var_idx: %ld\n", *read_write, *line_number, addr, *var_idx);
	exit(1);
}
