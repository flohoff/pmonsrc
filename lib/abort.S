/* $Id: abort.S,v 1.2 1996/01/16 14:17:35 chris Exp $ */
#include <regdef.h>

	.globl abort
	.ent abort
abort:
	break	0
	j	ra
	.end abort
