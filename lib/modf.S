/* $Id: modf.S,v 1.2 1996/01/16 14:18:14 chris Exp $ */
#include "mips.h"

	.sdata
one:	.double 1.0
maxint:	.double 4.503599627370496000E15

	.text
	.globl modf
	.ent modf
modf:
	l.d	$f6,maxint
	abs.d	$f0,$f12
	c.lt.d	$f0,$f6
	mfc1	t0,$f13
	bc1f	3f
	add.d	$f2,$f0,$f6
	sub.d	$f2,$f2,$f6
	c.le.d	$f2,$f0
	bc1t	1f
	l.d	$f6,one
	sub.d	$f2,$f2,$f6
1:	bgez	t0,2f
	neg.d	$f2,$f2

2:	s.d	$f2,0(a2)
	sub.d	$f0,$f12,$f2
	j	ra

3:	mtc1	zero,$f0
	mtc1	zero,$f1
	s.d	$f12,0(a2)
	j	ra
	.end modf


