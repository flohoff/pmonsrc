/* $Id: setbs.S,v 1.2 1996/01/16 14:18:26 chris Exp $ */
#include "mips.h"

/*
 * This is the module for setting the block refill size on the
 * HTP board. The block size is controlled by aux outputs on the
 * 2681 duart.
 */

#ifdef MIPSEB
#define SCCBASE 0xbe000003
#else
#define SCCBASE 0xbe000000
#endif

	SETOCR = 0x38
	CLROCR = 0x3c

	.globl setibs
	.ent setibs
setibs:	# values of a0 are: 0,1,2,3 = bs2,4,8,16
	# bits 1&0 in the OCR control ibs
	/*# 00 = bs16 01=bs8 10=bs4 11=bs2*/
	li	t1,SCCBASE
	move	t0,a0
	sb	t0,CLROCR(t1)
	not	a0
	and	a0,0x3		# mask
	sb	a0,SETOCR(t1)
	j	ra
	.end setibs

	.globl setdbs
	.ent setdbs
setdbs:	# values of a0 are: 0,1,2,3
	# bits 3&2 in the OCR control dbs
	/*# 00 = bs16 01=bs8 10=bs4 11=bs2*/
	li	t1,SCCBASE
	move	t0,a0
	sll	t0,2		# OCR bits 3&2
	sb	t0,CLROCR(t1)
	not	a0
	and	a0,0x3		# mask
	sll	a0,2		# OCR bits 3&2
	sb	a0,SETOCR(t1)
	j	ra
	.end setdbs

