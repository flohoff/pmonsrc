/* $Id: fabs.S,v 1.2 1996/01/16 14:17:49 chris Exp $ */
#include "mips.h"

	.globl fabs
	.ent fabs
fabs:
	abs.d	$f0,$f12
	j	ra
	.end fabs
