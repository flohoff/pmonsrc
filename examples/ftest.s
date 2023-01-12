/* $Id: ftest.s,v 1.2 1996/01/16 14:16:56 chris Exp $ */
#include <regdef.h>

/*************************************************************
*  _ftest
*	Part of the program fmain.c
*/

	.globl _ftest
	.ent _ftest
_ftest:
	li.s	$f6,1.0
	li.s	$f8,1.0
	add.s	$f4,$f6,$f8
	sub.s	$f4,$f6,$f8
	mul.s	$f4,$f6,$f8
	div.s	$f4,$f6,$f8
	mov.s	$f4,$f6
	neg.s	$f4,$f6
	abs.s	$f4,$f6
	c.eq.s	$f4,$f6

	li.d	$f6,1.0
	li.d	$f8,1.0
	add.d	$f4,$f6,$f8
	sub.d	$f4,$f6,$f8
	mul.d	$f4,$f6,$f8
	div.d	$f4,$f6,$f8
	mov.d	$f4,$f6
	neg.d	$f4,$f6
	abs.d	$f4,$f6
	c.eq.d	$f4,$f6

	cvt.d.s	$f4,$f6
	cvt.d.w	$f4,$f6
	cvt.w.s	$f4,$f6
	cvt.w.d	$f4,$f6
	cvt.s.d	$f4,$f6
	cvt.s.w	$f4,$f6

	cfc1	t0,C1_CSR
	ctc1	t0,C1_CSR
	la	t1,dat1
	lwc1	$f4,0(t1)
	swc1	$f4,0(t1)
	mfc1	t0,$f4
	mtc1	t0,$f4
	j	ra
	.end _ftest

	.data
dat1:	.float 2.0
