/* $Id: strcmp.S,v 1.3 1997/10/08 09:54:45 chris Exp $ */
#include <regdef.h>

#ifdef TEST
#define strcmp xstrcmp
#endif

/*
** Loop unrolled four times
*/

	.globl	strcmp
	.ent	strcmp
strcmp:
	.set noreorder
	.set nowarn
	
	# main loop
1:	lbu	t0, (a0)
	lbu	t1, (a1)
	beq	t0, zero, 2f	# brif end of string
	 lbu	t2, 1(a0)
	bne	t0, t1, 2f	# brif bytes not equal
	 lbu	t3, 1(a1)
	beq	t2, zero, 3f	# brif end of string
	 lbu	t4, 2(a0)
	bne	t2, t3, 3f	# brif bytes not equal
	 lbu	t5, 2(a1)
	beq	t4, zero, 4f	# brif end of string
	 lbu	t6, 3(a0)
	bne	t4, t5, 4f	# brif bytes not equal
	 lbu	t7, 3(a1)
	beq	t6, zero, 5f	# brif end of string
	 addu	a0, 4
	beq	t6, t7, 1b	# brif bytes equal
	 addu	a1, 4
5:	j	ra
	 subu	v0, t6, t7

2:	j	ra
	 subu	v0, t0, t1
3:	j	ra
	 subu	v0, t2, t3
4:	j	ra
	 subu	v0, t4, t5

	.set warn
	.set reorder
	.end	strcmp
