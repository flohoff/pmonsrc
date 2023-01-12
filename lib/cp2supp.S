/* $Id: cp2supp.S,v 1.3 1998/06/17 00:49:32 chris Exp $ */
#ifndef R4000

#ifndef LR33020
#define LR33020
#endif

#include "mips.h"

/*
** Support for LR33020's coprocessor 2
**
**	cp2init()
**	value = cfc2(reg)
**	ctc2(value,reg)
**	value = mfc2(reg)
**	mtc2(value,reg)
*/

	.globl cp2init
	.ent cp2init
cp2init:
	.set noreorder
	.set noat

	# enable the cp2
	mfc0    t0, C0_SR       # get status register
	lui     t1, 0x5000      # set CU2 Usable.and CU0
	or      t0, t0, t1      # set CU2 Usable.
	mtc0    t0, C0_SR

	# initialise dma registers
	la      t2, 0x8000000  #t2 <- addr where reg contents are stuffed
	mtc2    t2, C2_SCRSTART
	li      t2, 1024
	mtc2    t2, C2_SCRPITCH
	li      t2, 0x3fc
	mtc2    t2, C2_SAMEXTENT
	li      t2, 1024
	mtc2    t2, C2_LINECOUNT
	mtc2    zero, C2_HWCRSRSTART
	li      t4, 0x20
	mtc2    t4, C2_VBLKSIZE   

	#read video register data into cpu regs
	li      t0, 0x8000b300  # for vhwconfig
	li      t1, 0x0         # for hlintr
	li      t5, 0x001d0026  # for blank_end
	li      t6, 0x03170102  # for blank_start
	li      t7, 0x031c010c  # for sync_start
	li      t8, 0x03210134  # for sync_reset
	li      t3, 0xe4        # for serra

	#setup io2 and io3, turn DMA channels off
	li      t4, 0xffff7083  # ffe6ff83 to turn sam, hwcrsr on
	nop
	ctc2    t4, C2_CONFIG
	nop
	#write video registers from cpu regs
	#The following code shuld (preferably) be run from the IC
	.align 4
	li      s0, 0xa000a     # Init the cursor to line 10 pix 10
	mtc2    t3, C2_SERPULS
	nop
	mtc2    s0, C2_HWCRSR
	nop
	mtc2    t1, C2_HLINTR
	mtc2    t5, C2_BLANKE
	mtc2    t6, C2_BLANKS
	mtc2    t7, C2_SYNCS
	mtc2    t8, C2_SYNCRESET
	mtc2    t0, C2_VHWCONFIG

 # Also initialize the bbcp to to step instructions
	li	t0, 0xFFFFFFFF
	ctc2	zero, C2_SRCSKEW	# select src
	ctc2	zero, C2_SRCSHIFT
	ctc2	zero, C2_COLOR0
	ctc2	zero, C2_COLOR1
	li	t1, 0x3	
	ctc2	t1, C2_RASTEROP	# lu_op = src 
	ctc2	t0, C2_PLANEMASK
	mtc2	t0, C2_LEFTMASK
	mtc2	t0, C2_RIGHTMASK

	li	t0, 0xFFFFFDFF	# +x, +y, wo
	ctc2	t0, C2_GCPCNTRL
	li	t0, 0xaabbccdd
	mtc2	t0, C2_SRCDATA

	.set reorder
	.set at
	j	ra
	.end cp2init

	.data
cfc2L:	.word cfc20, cfc21, cfc22, cfc23, cfc24, cfc25, cfc26, cfc27
	.word cfc28, cfc29, cfc210, cfc211, cfc212, cfc213, cfc214
	.word cfc215, cfc216, cfc217, cfc218, cfc219, cfc220, cfc221
	.word cfc222, cfc223, cfc224, cfc225, cfc226, cfc227, cfc228
	.word cfc229, cfc230, cfc231

	.text
	.globl cfc2
	.ent cfc2
cfc2: 	# value = cfc2(reg)
	la	t0,cfc2L
	sll	a0,2
	addu	a0,t0
	lw	t0,(a0)
	j	t0

cfc20:	cfc2	v0,$0; b	1f
cfc21:	cfc2	v0,$1; b	1f
cfc22:	cfc2	v0,$2; b	1f
cfc23:	cfc2	v0,$3; b	1f
cfc24:	cfc2	v0,$4; b	1f
cfc25:	cfc2	v0,$5; b	1f
cfc26:	cfc2	v0,$6; b	1f
cfc27:	cfc2	v0,$7; b	1f
cfc28:	cfc2	v0,$8; b	1f
cfc29:	cfc2	v0,$9; b	1f
cfc210:	cfc2	v0,$10; b	1f
cfc211:	cfc2	v0,$11; b	1f
cfc212:	cfc2	v0,$12; b	1f
cfc213:	cfc2	v0,$13; b	1f
cfc214:	cfc2	v0,$14; b	1f
cfc215:	cfc2	v0,$15; b	1f
cfc216:	cfc2	v0,$16; b	1f
cfc217:	cfc2	v0,$17; b	1f
cfc218:	cfc2	v0,$18; b	1f
cfc219:	cfc2	v0,$19; b	1f
cfc220:	cfc2	v0,$20; b	1f
cfc221:	cfc2	v0,$21; b	1f
cfc222:	cfc2	v0,$22; b	1f
cfc223:	cfc2	v0,$23; b	1f
cfc224:	cfc2	v0,$24; b	1f
cfc225:	cfc2	v0,$25; b	1f
cfc226:	cfc2	v0,$26; b	1f
cfc227:	cfc2	v0,$27; b	1f
cfc228:	cfc2	v0,$28; b	1f
cfc229:	cfc2	v0,$29; b	1f
cfc230:	cfc2	v0,$30; b	1f
cfc231:	cfc2	v0,$31
1:	j	ra
	.end cfc2

	.data
ctc2L:	.word ctc20, ctc21, ctc22, ctc23, ctc24, ctc25, ctc26, ctc27
	.word ctc28, ctc29, ctc210, ctc211, ctc212, ctc213, ctc214
	.word ctc215, ctc216, ctc217, ctc218, ctc219, ctc220, ctc221
	.word ctc222, ctc223, ctc224, ctc225, ctc226, ctc227, ctc228
	.word ctc229, ctc230, ctc231

	.text
	.globl ctc2
	.ent ctc2
ctc2:	# ctc2(reg,value)
	la	t0,ctc2L
	sll	a0,2
	addu	a0,t0
	lw	t0,(a0)
	j	t0

ctc20:	ctc2	a1,$0; b	1f
ctc21:	ctc2	a1,$1; b	1f
ctc22:	ctc2	a1,$2; b	1f
ctc23:	ctc2	a1,$3; b	1f
ctc24:	ctc2	a1,$4; b	1f
ctc25:	ctc2	a1,$5; b	1f
ctc26:	ctc2	a1,$6; b	1f
ctc27:	ctc2	a1,$7; b	1f
ctc28:	ctc2	a1,$8; b	1f
ctc29:	ctc2	a1,$9; b	1f
ctc210:	ctc2	a1,$10; b	1f
ctc211:	ctc2	a1,$11; b	1f
ctc212:	ctc2	a1,$12; b	1f
ctc213:	ctc2	a1,$13; b	1f
ctc214:	ctc2	a1,$14; b	1f
ctc215:	ctc2	a1,$15; b	1f
ctc216:	ctc2	a1,$16; b	1f
ctc217:	ctc2	a1,$17; b	1f
ctc218:	ctc2	a1,$18; b	1f
ctc219:	ctc2	a1,$19; b	1f
ctc220:	ctc2	a1,$20; b	1f
ctc221:	ctc2	a1,$21; b	1f
ctc222:	ctc2	a1,$22; b	1f
ctc223:	ctc2	a1,$23; b	1f
ctc224:	ctc2	a1,$24; b	1f
ctc225:	ctc2	a1,$25; b	1f
ctc226:	ctc2	a1,$26; b	1f
ctc227:	ctc2	a1,$27; b	1f
ctc228:	ctc2	a1,$28; b	1f
ctc229:	ctc2	a1,$29; b	1f
ctc230:	ctc2	a1,$30; b	1f
ctc231:	ctc2	a1,$31
1:	j	ra
	.end ctc2

	.data
mfc2L:	.word mfc20, mfc21, mfc22, mfc23, mfc24, mfc25, mfc26, mfc27
	.word mfc28, mfc29, mfc210, mfc211, mfc212, mfc213, mfc214
	.word mfc215, mfc216, mfc217, mfc218, mfc219, mfc220, mfc221
	.word mfc222, mfc223, mfc224, mfc225, mfc226, mfc227, mfc228
	.word mfc229, mfc230, mfc231

	.text
	.globl mfc2
	.ent mfc2
mfc2:	# value = mfc2(reg)
	la	t0,mfc2L
	sll	a0,2
	addu	a0,t0
	lw	t0,(a0)
	j	t0

mfc20:	mfc2	v0,$0; b	1f
mfc21:	mfc2	v0,$1; b	1f
mfc22:	mfc2	v0,$2; b	1f
mfc23:	mfc2	v0,$3; b	1f
mfc24:	mfc2	v0,$4; b	1f
mfc25:	mfc2	v0,$5; b	1f
mfc26:	mfc2	v0,$6; b	1f
mfc27:	mfc2	v0,$7; b	1f
mfc28:	mfc2	v0,$8; b	1f
mfc29:	mfc2	v0,$9; b	1f
mfc210:	mfc2	v0,$10; b	1f
mfc211:	mfc2	v0,$11; b	1f
mfc212:	mfc2	v0,$12; b	1f
mfc213:	mfc2	v0,$13; b	1f
mfc214:	mfc2	v0,$14; b	1f
mfc215:	mfc2	v0,$15; b	1f
mfc216:	mfc2	v0,$16; b	1f
mfc217:	mfc2	v0,$17; b	1f
mfc218:	mfc2	v0,$18; b	1f
mfc219:	mfc2	v0,$19; b	1f
mfc220:	mfc2	v0,$20; b	1f
mfc221:	mfc2	v0,$21; b	1f
mfc222:	mfc2	v0,$22; b	1f
mfc223:	mfc2	v0,$23; b	1f
mfc224:	mfc2	v0,$24; b	1f
mfc225:	mfc2	v0,$25; b	1f
mfc226:	mfc2	v0,$26; b	1f
mfc227:	mfc2	v0,$27; b	1f
mfc228:	mfc2	v0,$28; b	1f
mfc229:	mfc2	v0,$29; b	1f
mfc230:	mfc2	v0,$30; b	1f
mfc231:	mfc2	v0,$31 
1:	j	ra
	.end mfc2

	.data
mtc2L:	.word mtc20, mtc21, mtc22, mtc23, mtc24, mtc25, mtc26, mtc27
	.word mtc28, mtc29, mtc210, mtc211, mtc212, mtc213, mtc214
	.word mtc215, mtc216, mtc217, mtc218, mtc219, mtc220, mtc221
	.word mtc222, mtc223, mtc224, mtc225, mtc226, mtc227, mtc228
	.word mtc229, mtc230, mtc231

	.text
	.globl mtc2
	.ent mtc2
mtc2:	# mtc2(reg,value)
	la	t0,mtc2L
	sll	a0,2
	addu	a0,t0
	lw	t0,(a0)
	j	t0

mtc20:	mtc2	a1,$0; b	1f
mtc21:	mtc2	a1,$1; b	1f
mtc22:	mtc2	a1,$2; b	1f
mtc23:	mtc2	a1,$3; b	1f
mtc24:	mtc2	a1,$4; b	1f
mtc25:	mtc2	a1,$5; b	1f
mtc26:	mtc2	a1,$6; b	1f
mtc27:	mtc2	a1,$7; b	1f
mtc28:	mtc2	a1,$8; b	1f
mtc29:	mtc2	a1,$9; b	1f
mtc210:	mtc2	a1,$10; b	1f
mtc211:	mtc2	a1,$11; b	1f
mtc212:	mtc2	a1,$12; b	1f
mtc213:	mtc2	a1,$13; b	1f
mtc214:	mtc2	a1,$14; b	1f
mtc215:	mtc2	a1,$15; b	1f
mtc216:	mtc2	a1,$16; b	1f
mtc217:	mtc2	a1,$17; b	1f
mtc218:	mtc2	a1,$18; b	1f
mtc219:	mtc2	a1,$19; b	1f
mtc220:	mtc2	a1,$20; b	1f
mtc221:	mtc2	a1,$21; b	1f
mtc222:	mtc2	a1,$22; b	1f
mtc223:	mtc2	a1,$23; b	1f
mtc224:	mtc2	a1,$24; b	1f
mtc225:	mtc2	a1,$25; b	1f
mtc226:	mtc2	a1,$26; b	1f
mtc227:	mtc2	a1,$27; b	1f
mtc228:	mtc2	a1,$28; b	1f
mtc229:	mtc2	a1,$29; b	1f
mtc230:	mtc2	a1,$30; b	1f
mtc231:	mtc2	a1,$31 
1:	j	ra
	.end mtc2

#endif /* !R4000 */
