/* $Id: cp0supp.S,v 1.2 1996/01/16 14:17:44 chris Exp $ */
#include "mips.h"

/*
** Support for Rx000 coprocessor 0 registers
**
**	value = mfc0(reg)
**	mfc0(value,reg)
*/

	.rdata
	.data
mfc0L:	.word mfc00, mfc01, mfc02, mfc03, mfc04, mfc05, mfc06, mfc07
	.word mfc08, mfc09, mfc010, mfc011, mfc012, mfc013, mfc014
	.word mfc015, mfc016, mfc017, mfc018, mfc019, mfc020, mfc021
	.word mfc022, mfc023, mfc024, mfc025, mfc026, mfc027, mfc028
	.word mfc029, mfc030, mfc031

	.text
	.globl mfc0
	.ent mfc0
mfc0:	# value = mfc0(reg)
	la	t0,mfc0L
	sll	a0,2
	addu	a0,t0
	lw	t0,(a0)
	j	t0

mfc00:	mfc0	v0,$0; b	1f
mfc01:	mfc0	v0,$1; b	1f
mfc02:	mfc0	v0,$2; b	1f
mfc03:	mfc0	v0,$3; b	1f
mfc04:	mfc0	v0,$4; b	1f
mfc05:	mfc0	v0,$5; b	1f
mfc06:	mfc0	v0,$6; b	1f
mfc07:	mfc0	v0,$7; b	1f
mfc08:	mfc0	v0,$8; b	1f
mfc09:	mfc0	v0,$9; b	1f
mfc010:	mfc0	v0,$10; b	1f
mfc011:	mfc0	v0,$11; b	1f
mfc012:	mfc0	v0,$12; b	1f
mfc013:	mfc0	v0,$13; b	1f
mfc014:	mfc0	v0,$14; b	1f
mfc015:	mfc0	v0,$15; b	1f
mfc016:	mfc0	v0,$16; b	1f
mfc017:	mfc0	v0,$17; b	1f
mfc018:	mfc0	v0,$18; b	1f
mfc019:	mfc0	v0,$19; b	1f
mfc020:	mfc0	v0,$20; b	1f
mfc021:	mfc0	v0,$21; b	1f
mfc022:	mfc0	v0,$22; b	1f
mfc023:	mfc0	v0,$23; b	1f
mfc024:	mfc0	v0,$24; b	1f
mfc025:	mfc0	v0,$25; b	1f
mfc026:	mfc0	v0,$26; b	1f
mfc027:	mfc0	v0,$27; b	1f
mfc028:	mfc0	v0,$28; b	1f
mfc029:	mfc0	v0,$29; b	1f
mfc030:	mfc0	v0,$30; b	1f
mfc031:	mfc0	v0,$31 
1:	j	ra
	.end mfc0

	.rdata
mtc0L:	.word mtc00, mtc01, mtc02, mtc03, mtc04, mtc05, mtc06, mtc07
	.word mtc08, mtc09, mtc010, mtc011, mtc012, mtc013, mtc014
	.word mtc015, mtc016, mtc017, mtc018, mtc019, mtc020, mtc021
	.word mtc022, mtc023, mtc024, mtc025, mtc026, mtc027, mtc028
	.word mtc029, mtc030, mtc031

	.text
	.globl mtc0
	.ent mtc0
mtc0:	# mtc0(reg,value)
	la	t0,mtc0L
	sll	a0,2
	addu	a0,t0
	lw	t0,(a0)
	j	t0

mtc00:	mtc0	a1,$0; b	1f
mtc01:	mtc0	a1,$1; b	1f
mtc02:	mtc0	a1,$2; b	1f
mtc03:	mtc0	a1,$3; b	1f
mtc04:	mtc0	a1,$4; b	1f
mtc05:	mtc0	a1,$5; b	1f
mtc06:	mtc0	a1,$6; b	1f
mtc07:	mtc0	a1,$7; b	1f
mtc08:	mtc0	a1,$8; b	1f
mtc09:	mtc0	a1,$9; b	1f
mtc010:	mtc0	a1,$10; b	1f
mtc011:	mtc0	a1,$11; b	1f
mtc012:	mtc0	a1,$12; b	1f
mtc013:	mtc0	a1,$13; b	1f
mtc014:	mtc0	a1,$14; b	1f
mtc015:	mtc0	a1,$15; b	1f
mtc016:	mtc0	a1,$16; b	1f
mtc017:	mtc0	a1,$17; b	1f
mtc018:	mtc0	a1,$18; b	1f
mtc019:	mtc0	a1,$19; b	1f
mtc020:	mtc0	a1,$20; b	1f
mtc021:	mtc0	a1,$21; b	1f
mtc022:	mtc0	a1,$22; b	1f
mtc023:	mtc0	a1,$23; b	1f
mtc024:	mtc0	a1,$24; b	1f
mtc025:	mtc0	a1,$25; b	1f
mtc026:	mtc0	a1,$26; b	1f
mtc027:	mtc0	a1,$27; b	1f
mtc028:	mtc0	a1,$28; b	1f
mtc029:	mtc0	a1,$29; b	1f
mtc030:	mtc0	a1,$30; b	1f
mtc031:	mtc0	a1,$31 
1:	j	ra
	.end mtc0
