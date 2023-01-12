/* $Id: fpstate.S,v 1.2 1996/01/16 14:17:56 chris Exp $ */
#include "mips.h"

/*************************************************************
*  fpstatesz & fpstate
*	Used by clients who wish to call the PMON resident routines
*	of the same name.
*/

	.globl fpstatesz
	.ent fpstatesz
fpstatesz:
	li	t0,0xbfc00200
	lw	t0,13*4(t0)
	j	t0
	.end fpstatesz

	.globl fpstate
	.ent fpstate
fpstate: # fpstate(int op,long *p) op=0=get op=1=put
	li	t0,0xbfc00200
	lw	t0,15*4(t0)
	j	t0
	.end fpstate

