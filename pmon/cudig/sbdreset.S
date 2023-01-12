/*
** Copyright 1999 Algorithmics Ltd
**	All Rights Reserved
**	
** cudig/sbdreset.sx -- low level board dependent routines
*/

#include <mips.h>
#include <pmon.h>
#include <sbd.h>
#include <gt64011.h>
#include <mips/prid.h>
			
#define MB	0x100000
				
#if #endian(big)	
#define HTOLL(sr,tr) \
	.set noat ; \
	srl	AT,sr,24 ; \
	srl	tr,sr,8 ; \
	and	tr,0xff00 ; \
	or	AT,tr ; \
	and	tr,sr,0xff00 ; \
	sll	tr,8 ; \
	or	AT,tr ; \
	sll	tr,sr,24 ; \
	or	sr,AT,tr ; \
	.set at
#else
#define HTOLL(sr,tr)
#endif
	
#define DISPLAY(str) 						\
	li	a0,str;						\
	bal	sbddisplay
	
/* register usage in module */	
#define rasave	s0
#define p64011	s1
#define bank0	s2
#define bank1	s3
#define bank2	s4
#define bank3	s5
#define memtop	s6
#define membase	t9

/* bank type flags */	
#define MemTypeNone		0x8000
#define MemRasMask		0x0f00
#define MemRasShift		8
#define MemCasMask		0x000f
#define MemCasShift 		0

LEAF(sbdreset)
	mfc0	t2,C0_PRID
	move	rasave,ra		# save return address
	srl	t2,8
	bne	t2,PRID_R4650,2f
	
	/* r4640/50 initialisation */
	mtc0	zero,$0			# C0_IBASE
	mtc0	zero,$1			# C0_IBOUND
	mtc0	zero,$2			# C0_DBASE
	mtc0	zero,$3			# C0_DBOUND
	mtc0	zero,$18		# C0_IWATCH
	mtc0	zero,$19		# C0_DWATCH
#ifndef ITBASE	
	li	t1,0x22233333
	mtc0	t1,$17			# C0_CALG
#endif	
	b	3f

2:	
#ifndef ITBASE	
	/* set config register for 32b/32b cachelines, kseg0 cacheable */
	mfc0	t1,C0_CONFIG
	and	t1,~0x3f		# set bits 5..0 only
	or	t1,CFG_IB | CFG_DB | CFG_C_NONCOHERENT
	mtc0	t1,C0_CONFIG
#endif	
	
3:	
	/*
	 * slight amount of kludgery here to stop RAM resident
	 * version of PMON from overwriting itself...
	 */
	li	v1,0x1fc00000		/* check return address is in ROM */
	and	v0,ra,v1
	bne	v0,v1,.noinit

	/* table driven hardware register initialisation */
	la	a0, reginittab
	or	a0, K1BASE		/* force to kseg1 */

1:	lw	v0,0(a0)
	lw	v1,4(a0)
	addu	a0,8
	beqz	v0,8f
	sw	v1,0(v0)
	sync
	b	1b

8:
	/* initialise 4 character display */
	li	a0,DISPLAY0_IDLE
	sh	a0,PA_TO_KVA1(DISPLAY0_BASE)
	li	a0,DISPLAY1_IDLE
	sh	a0,PA_TO_KVA1(DISPLAY1_BASE)
	DISPLAY("INIT")
	
	li	p64011, PA_TO_KVA1(GT64011_BASE)
			
	/* configure DRAM
 	 * 
	 * each bank can be configured as 16Mx32 interleaved (128Mb total)
	 * Due to address space limitations we map each bank in turn to
	 * determine whether the slot is populated and what size RAMS are
	 * used.
         */

	/* set bank parameters */
	
/* base DRAM timing/parameter  setup */
#define GT_DRAMPAR_init \
			GT_DRAMPAR_CASWr2 | GT_DRAMPAR_RAStoCASWr2 | \
			GT_DRAMPAR_CASRd2 | GT_DRAMPAR_RAStoCASRd2 | \
			GT_DRAMPAR_ParityDisable
	
	DISPLAY("DPAR")
	
	li	t0, htoll(GT_DRAMPAR_init | GT_DRAMPAR_BankWidth64 | GT_DRAMPAR_Refresh4096)

	sw	t0,GT_DRAMPAR_BANK0(p64011)
	sw	t0,GT_DRAMPAR_BANK1(p64011)
	sw	t0,GT_DRAMPAR_BANK2(p64011)
	sw	t0,GT_DRAMPAR_BANK3(p64011)
	

	/* first for the banks 1 & 0 */

	/* enable ras1:0 decode */
	li	t0, htoll((DRAM_BASE +    0*MB) >> 21 & GT_PAS_LOMASK_Low)
	li	t1, htoll((DRAM_BASE +  128*MB - 1) >> 21 & GT_PAS_HIMASK_High)
	sw	t0, GT_PAS_RAS10LO(p64011)
	sw	t1, GT_PAS_RAS10HI(p64011)
	
	/* enable bank 0 */
	li	t0, htoll(((DRAM_BASE +   0*MB) >> 20) & GT_DDAS_LOMASK_Low)
	li	t1, htoll(((DRAM_BASE + 128*MB - 1) >> 20) & GT_DDAS_HIMASK_High)
	sw	t0, GT_DDAS_RAS0LO(p64011)
	sw	t1, GT_DDAS_RAS0HI(p64011)
		
	DISPLAY("TYP0")
	bal	memType
	move	bank0, v0

	/* disable bank 0, enable bank1 */
	li	t0, htoll(GT_DDAS_LOMASK_Low)
	li	t1, htoll(0)
	sw	t0, GT_DDAS_RAS0LO(p64011)
	sw	t1, GT_DDAS_RAS0HI(p64011)
	li	t0, htoll(((DRAM_BASE +   0*MB) >> 20) & GT_DDAS_LOMASK_Low)
	li	t1, htoll(((DRAM_BASE + 128*MB - 1) >> 20) & GT_DDAS_HIMASK_High)
	sw	t0, GT_DDAS_RAS1LO(p64011)
	sw	t1, GT_DDAS_RAS1HI(p64011)
		
	DISPLAY("TYP1")
	bal	memType
	move	bank1, v0

	/* disable bank 1 */
	li	t0, htoll(GT_DDAS_LOMASK_Low)
	li	t1, htoll(0)
	sw	t0, GT_DDAS_RAS1LO(p64011)
	sw	t1, GT_DDAS_RAS1HI(p64011)

		
	/* disable ras1:0 decode */
	li	t0, htoll(GT_PAS_LOMASK_Low)
	li	t1, htoll(0)
	sw	t0, GT_PAS_RAS10LO(p64011)
	sw	t1, GT_PAS_RAS10HI(p64011)


	/* now banks 3 & 2 */

	/* enable ras3:2 decode */
	li	t0, htoll((DRAM_BASE +    0*MB) >> 21 & GT_PAS_LOMASK_Low)
	li	t1, htoll((DRAM_BASE +  128*MB - 1) >> 21 & GT_PAS_HIMASK_High)
	sw	t0, GT_PAS_RAS32LO(p64011)
	sw	t1, GT_PAS_RAS32HI(p64011)
	
	/* enable bank 2 */
	li	t0, htoll(((DRAM_BASE +   0*MB) >> 20) & GT_DDAS_LOMASK_Low)
	li	t1, htoll(((DRAM_BASE + 128*MB - 1) >> 20) & GT_DDAS_HIMASK_High)
	sw	t0, GT_DDAS_RAS2LO(p64011)
	sw	t1, GT_DDAS_RAS2HI(p64011)
	
	DISPLAY("TYP2")
	bal	memType
	move	bank2, v0

	/* disable bank 2, enable bank3 */
	li	t0, htoll(GT_DDAS_LOMASK_Low)
	li	t1, htoll(0)
	sw	t0, GT_DDAS_RAS2LO(p64011)
	sw	t1, GT_DDAS_RAS2HI(p64011)
	li	t0, htoll(((DRAM_BASE +   0*MB) >> 20) & GT_DDAS_LOMASK_Low)
	li	t1, htoll(((DRAM_BASE + 128*MB - 1) >> 20) & GT_DDAS_HIMASK_High)
	sw	t0, GT_DDAS_RAS3LO(p64011)
	sw	t1, GT_DDAS_RAS3HI(p64011)
		
	DISPLAY("TYP3")
	bal	memType
	move	bank3, v0

	/* disable bank 3 */
	li	t0, htoll(GT_DDAS_LOMASK_Low)
	li	t1, htoll(0)
	sw	t0, GT_DDAS_RAS3LO(p64011)
	sw	t1, GT_DDAS_RAS3HI(p64011)

	/* disable ras3:2 decode */
	li	t0, htoll(GT_PAS_LOMASK_Low)
	li	t1, htoll(0)
	sw	t0, GT_PAS_RAS32LO(p64011)
	sw	t1, GT_PAS_RAS32HI(p64011)


	/* set new parameters based on results */
	li	memtop,DRAM_BASE 
	move	membase,memtop
	
	DISPLAY("PAR0")
	move	a0,bank0
	bal	memParm
	sw	v0,GT_DRAMPAR_BANK0(p64011)
	beqz	v1,1f
	
	srl	t0,memtop,20
	and	t0,GT_DDAS_LOMASK_Low
	HTOLL(t0,t2)
	addu	memtop,v1
	subu	t1,memtop,1
	srl	t1,20
	and	t1,GT_DDAS_HIMASK_High
	HTOLL(t1,t2)
	sw	t0, GT_DDAS_RAS0LO(p64011)
	sw	t1, GT_DDAS_RAS0HI(p64011)
	
	
1:	DISPLAY("PAR1")
	move	a0,bank1
	bal	memParm
	sw	v0,GT_DRAMPAR_BANK1(p64011)
	beqz	v1,1f

	srl	t0,memtop,20
	and	t0,GT_DDAS_LOMASK_Low
	HTOLL(t0,t2)
	addu	memtop,v1
	subu	t1,memtop,1
	srl	t1,20
	and	t1,GT_DDAS_HIMASK_High
	HTOLL(t1,t2)
	sw	t0, GT_DDAS_RAS1LO(p64011)
	sw	t1, GT_DDAS_RAS1HI(p64011)
	
1:
	/* enable ras1:0 decode if appropriate */
	beq	membase,memtop,1f


	DISPLAY("RAS0")
	
	srl	t0, membase, 21
	and	t0, GT_PAS_LOMASK_Low
	HTOLL(t0,t2)
	subu	t1, memtop, 1
	srl	t1, 21
	and	t1, GT_PAS_HIMASK_High
	HTOLL(t1,t2)
	sw	t0, GT_PAS_RAS10LO(p64011)
	sw	t1, GT_PAS_RAS10HI(p64011)

	move	membase,memtop

	/* same again for banks 3:2 */
	
1:	DISPLAY("PAR2")
	move	a0,bank2
	bal	memParm
	sw	v0,GT_DRAMPAR_BANK2(p64011)
	beqz	v1,1f
	
	srl	t0,memtop,20
	and	t0,GT_DDAS_LOMASK_Low
	HTOLL(t0,t2)
	addu	memtop,v1
	subu	t1,memtop,1
	srl	t1,20
	and	t1,GT_DDAS_HIMASK_High
	HTOLL(t1,t2)
	sw	t0, GT_DDAS_RAS2LO(p64011)
	sw	t1, GT_DDAS_RAS2HI(p64011)
	
1:	DISPLAY("PAR3")
	move	a0,bank3
	bal	memParm
	sw	v0,GT_DRAMPAR_BANK3(p64011)
	beqz	v1,1f

	srl	t0,memtop,20
	and	t0,GT_DDAS_LOMASK_Low
	HTOLL(t0,t2)
	addu	memtop,v1
	subu	t1,memtop,1
	srl	t1,20
	and	t1,GT_DDAS_HIMASK_High
	HTOLL(t1,t2)
	sw	t0, GT_DDAS_RAS3LO(p64011)
	sw	t1, GT_DDAS_RAS3HI(p64011)
	
1:
	/* enable ras3:2 decode if appropriate */
	beq	membase,memtop,1f

	DISPLAY("RAS2")
	
	srl	t0, membase, 21
	and	t0, GT_PAS_LOMASK_Low
	HTOLL(t0,t2)
	subu	t1, memtop, 1
	srl	t1, 21
	and	t1, GT_PAS_HIMASK_High
	HTOLL(t1,t2)
	sw	t0, GT_PAS_RAS32LO(p64011)
	sw	t1, GT_PAS_RAS32HI(p64011)


1:	
#ifdef DBGSBD	
	li	a0,PA_TO_KVA1(0x1fff0)
	sw	bank0,0(a0)
	sw	bank1,4(a0)
	sw	bank2,8(a0)
	sw	bank3,12(a0)
	sw	memtop,16(a0)
#endif
		
.noinit:
#if 0	
	/* initialise tlb */
	SBD_DISPLAY ('I','T','L','B', CHKPNT_ITLB)
	bal	init_tlb
#endif
	move	ra,rasave
	j	ra
END(sbdreset)
	

LEAF(_sbd_memfail)
	SBD_DISPLAY ('!','M','E','M',CHKPNT_0MEM)
1:	b	1b
	j	ra
END(_sbd_memfail)
	

	
	.rdata
RefreshBits:
	.word	htoll(GT_DRAMPAR_Refresh512)
	.word	htoll(GT_DRAMPAR_Refresh1024)
	.word	htoll(GT_DRAMPAR_Refresh2048)
	.word	htoll(GT_DRAMPAR_Refresh4096)
	.text

SLEAF(memParm)
	li	v0, htoll(GT_DRAMPAR_init)
	li	v1, 0
	
	srl	a1,a0,16
	
	and	t0,a0,a1
	and	t0,MemTypeNone
	bnez	t0,9f		# no memory

	or	t0,a0,a1
	and	t0,MemTypeNone
	bnez	t0,.onebank	# only one bank
	
	and	t0,a0,(~MemTypeNone) & 0xffff
	and	t1,a1,(~MemTypeNone) & 0xffff
	bne	t0,t1,.diffbank
	
	/*
	 * both banks are populated with the same size of DRAM
	 * so they can be interleaved
	 */

	or	v0,htoll(GT_DRAMPAR_BankWidth64)

	b	.finish

.onebank:	/* One of the banks isn't populated - decide which one */
	or	v0,htoll(GT_DRAMPAR_BankWidth32)
	/*
	 * The order in which the banks are accessed is CPU endianess
	 * dependent.  The Galileo documentation describes little-endian
	 * behaviour
	 */
#if #endian(big)
#define GT_DRAMPAR_BankLocLo	GT_DRAMPAR_BankLocOdd
#define GT_DRAMPAR_BankLocHi	GT_DRAMPAR_BankLocEven
#else
#define GT_DRAMPAR_BankLocLo	GT_DRAMPAR_BankLocEven
#define GT_DRAMPAR_BankLocHi	GT_DRAMPAR_BankLocOdd
#endif		
	li	t1,htoll(GT_DRAMPAR_BankLocLo)
	move	t0,a0
	and	t2,a0,MemTypeNone
	beqz	t2,1f
	li	t1,htoll(GT_DRAMPAR_BankLocHi)
	move	t0,a1
1:	or	v0,t1
	
	b	.finish
	
.diffbank:
	/* choose the one with the most row and column bits */
	or	v0,htoll(GT_DRAMPAR_BankWidth32)
	li	t1,htoll(GT_DRAMPAR_BankLocLo)
	move	t0,a0
	and	t2,a0,MemRasMask|MemCasMask
	and	t3,a1,MemRasMask|MemCasMask
	bgeu	t2,t3,1f
	li	t1,htoll(GT_DRAMPAR_BankLocHi)
	move	t0,a1
1:	or	v0,t1

	/* b .finish */

.finish:
	/* add the refresh bits */
	and	t1,t0,MemRasMask
	srl	t1,MemRasShift
	subu	t1,9
	sll	t1,2
	la	t2,RefreshBits
	or	t2,K1BASE
	addu	t1,t2
	lw	t1,0(t1)
	or	v0,t1

	/* add the number of RAS & CAS bits together to get the total size */
	and	t1,t0,MemCasMask
	srl	t1,MemCasShift
	and	t0,MemRasMask
	srl	t0,MemRasShift
	addu	t0,t1

	/* set memory size based on number of address bits */
	li	v1,4
	sll	v1,t0

	/* memory size is doubled if interleaved */
	and	t0,v0,htoll(GT_DRAMPAR_BankWidth64)
	beqz	t0,9f
	sll	v1,1
	
9:	j	ra
SEND(memParm)
		
SLEAF(memType)
	/* prime the banks by doing 8 read cycles */
	li	a0,PA_TO_KVA1(DRAM_BASE +  0*MB)
	li	t0,8
1:	lw	zero, 0(a0)
	lw	zero, 4(a0)
	subu	t0,1
	bnez	t0,1b
	
	/*
	 * The bank is configured as 16Mx32 interleaved
	 * Write at predetermined address to see which RAS lines are unused
	 */

	li	t1,12 << MemRasShift
	addu	t0,a0,0
	sw	t1,0(t0)
	sw	t1,4(t0)

	li	t1,11 << MemRasShift
	addu	t0,a0,1 << 16
	sw	t1,0(t0)
	sw	t1,4(t0)

	li	t1,10 << MemRasShift
	addu	t0,a0,1 << 15
	sw	t1,0(t0)
	sw	t1,4(t0)

	li	t1,9 << MemRasShift
	addu	t0,a0,1 << 14
	sw	t1,0(t0)
	sw	t1,4(t0)

	/* make sure the memory is really there */
	sw	a0,8(a0)
	sw	a0,12(a0)
	not	t0,a0
	sw	t0,16(a0)
	sw	t0,20(a0)
	
	li	v0,MemTypeNone
	lw	t0,8(a0)
	bne	a0,t0,1f
	lw	v0,0(a0)
1:

	li	v1,MemTypeNone
	lw	t0,12(a0)
	bne	a0,t0,1f
	lw	v1,4(a0)
1:	sll	v1,16
	or	v0,v1
		
	/* now determine the CAS sizes */

	li	t0,12
	sw	t0,0(a0)
	sw	t0,4(a0)

	li	t1,11
	addu	t0,a0,1 << 26
	sw	t1,0(t0)
	sw	t1,4(t0)

	li	t1,10
	addu	t0,a0,1 << 25
	sw	t1,0(t0)
	sw	t1,4(t0)

	li	t1,9
	addu	t0,a0,1 << 24
	sw	t1,0(t0)
	sw	t1,4(t0)

	and	t0,v0,MemTypeNone
	bnez	t0,1f
	lw	v1,0(a0)
	or	v0,v1
1:	and	t0,v0,MemTypeNone<<16
	bnez	t0,1f
	lw	v1,4(a0)
	sll	v1,16
	or	v0,v1
1:
			
9:	j	ra		
SEND(memType)
	
		

/* DRAM: */	
#define DRAM_CFG_init \
	GT_DRAM_CFG_RefIntCnt(512) | \
	GT_DRAM_CFG_StagRefOn | \
	GT_DRAM_CFG_ADSFunctDRAM | \
	GT_DRAM_CFG_DRAMLatchActive
	
/* basic i/o decode settings: widest timings, latch enabled, no parity */
#define DEVPAR_init \
	GT_DEVPAR_TurnOff(7) | \
	GT_DEVPAR_AccToFirst(15) | \
	GT_DEVPAR_AccToNext(15) | \
	GT_DEVPAR_ADStoWr(7) | \
	GT_DEVPAR_WrActive(7) | \
	GT_DEVPAR_WrHigh(7) | \
	GT_DEVPAR_LatchFunctEnable /* XXX GT_DEVPAR_LatchFunctTransparent */ | \
	GT_DEVPAR_ParityDisable | \
	GT_DEVPAR_Reserved
	
/* CS0 (HPI) 16 bit bus, odd */
#define DEVPAR_CS0 (DEVPAR_init | GT_DEVPAR_DevWidth16 | GT_DEVPAR_DevLocOdd)
	
/* CS1 (RF card #1) 16 bit bus, odd */
#define DEVPAR_CS1 (DEVPAR_init | GT_DEVPAR_DevWidth16 | GT_DEVPAR_DevLocOdd)
	
/* CS2 (misc) 16 bit bus, odd */
#define DEVPAR_CS2 (DEVPAR_init | GT_DEVPAR_DevWidth16 | GT_DEVPAR_DevLocOdd)
	
/* CS3 (misc) 32 bit bus, even */
#define DEVPAR_CS3 (DEVPAR_init | GT_DEVPAR_DevWidth32 | GT_DEVPAR_DevLocEven)
	
/* BOOTCS (EPROM & UART) 8 bit bus, even */
#define DEVPAR_BOOT (GT_DEVPAR_DevWidth8 | GT_DEVPAR_DevLocEven | \
	GT_DEVPAR_AccToFirst(10) | \
	GT_DEVPAR_AccToNext(9) | \
	GT_DEVPAR_TurnOff(6) | \
	GT_DEVPAR_ADStoWr(7) | \
	GT_DEVPAR_WrActive(7) | \
	GT_DEVPAR_WrHigh(7) | \
	GT_DEVPAR_LatchFunctEnable | \
	GT_DEVPAR_ParityDisable | \
	GT_DEVPAR_Reserved)

#define INIT(addr,val) \
	.word	addr, val
	
#define GTINIT(offs,val) \
	INIT(PHYS_TO_K1(GT64011_BASE+(offs)), htoll(val))
	
	.rdata
reginittab:	
	/* program non-default local device address space decode */
	GTINIT(GT_PAS_CS3BOOTLO, (CS3_BASE >> 21) & 0x7ff)
	GTINIT(GT_PAS_CS3BOOTHI, (BOOTCS_LIMIT >> 21) & 0x7f)
	GTINIT(GT_DDAS_CS3LO,	 (CS3_BASE >> 20) & 0xff)
	GTINIT(GT_DDAS_CS3HI,	 (CS3_LIMIT >> 20) & 0xff)
	GTINIT(GT_DDAS_BOOTCSLO, (BOOTCS_BASE >> 20) & 0xff)
	GTINIT(GT_DDAS_BOOTCSHI, (BOOTCS_LIMIT >> 20) & 0xff)
	
	/* setup local device timing parameters */
	GTINIT(GT_DEVPAR_BANK0, DEVPAR_CS0)
	GTINIT(GT_DEVPAR_BANK1, DEVPAR_CS1)
	GTINIT(GT_DEVPAR_BANK2, DEVPAR_CS2)
	GTINIT(GT_DEVPAR_BANK3, DEVPAR_CS3)
	GTINIT(GT_DEVPAR_BOOT,  DEVPAR_BOOT)
	
	/* disable ras1:0 and ras3:2 decodes */
	GTINIT(GT_PAS_RAS10LO,	GT_PAS_LOMASK_Low)
	GTINIT(GT_PAS_RAS10HI,	0)
	GTINIT(GT_PAS_RAS32LO,	GT_PAS_LOMASK_Low)
	GTINIT(GT_PAS_RAS32HI,	0)
	
	/* disable RAS[0123] */
	GTINIT(GT_DDAS_RAS0LO,	GT_DDAS_LOMASK_Low)
	GTINIT(GT_DDAS_RAS0HI,	0)
	GTINIT(GT_DDAS_RAS1LO,	GT_DDAS_LOMASK_Low)
	GTINIT(GT_DDAS_RAS1HI,	0)
	GTINIT(GT_DDAS_RAS2LO,	GT_DDAS_LOMASK_Low)
	GTINIT(GT_DDAS_RAS2HI,	0)
	GTINIT(GT_DDAS_RAS3LO,	GT_DDAS_LOMASK_Low)
	GTINIT(GT_DDAS_RAS3HI,	0)
	
	/* initialise DRAM timing */
	GTINIT(GT_DRAM_CFG,	DRAM_CFG_init)
	
	/* initialise contro1 port 1 (pulse serial port reset) */
	INIT(PHYS_TO_K1(CONTROL1_BASE), CONTROL1_SER2_RST | CONTROL1_SER1_RST)
	/* initialise contro1 port 1 (unreset flash & serial ports) */
	INIT(PHYS_TO_K1(CONTROL1_BASE), CONTROL1_FLASH_RST_N)
	
	INIT(0,0)
	.text


	
	.globl sbddelay
LEAF(sbdberrenb)
	mfc0	v0,C0_SR
	li	t0,SR_DE	
	bnez	a0,1f
	or	t1,v0,t0	# disable cache/parity errors (SR_DE = 1)
	b	2f
1:	not	t1,t0		# enable cache/parity errors (SR_DE = 0)
	and	t1,v0
2:	mtc0	t1,C0_SR
	and	v0,t0		# get old SR_DE bit
	xor	v0,t0		# and invert to make it an enable bit
	j	ra
END(sbdberrenb)


LEAF(sbdberrcnt)
	move	v0,zero
	j	ra
END(sbdberrcnt)
	

/* 
 * Display 4 character string on "dot matrix" display
 */
LEAF(sbddisplay)
	li	t0,PA_TO_KVA1(DISPLAY0_BASE)
	li	t1,PA_TO_KVA1(DISPLAY1_BASE)
	/* start address 0 + default state */
	li	t3,DISPLAY1_IDLE
88:	/* set CE1~ & CE2~ low */
	and	t3,~(DISPLAY1_NCE1 | DISPLAY1_NCE2)
	sh	t3,0(t1)
	/* write character (CLR~ high) */
	or	a0,DISPLAY0_NCLR
	sh	a0,0(t0)
	/* set WR~ low */
	and	t3,~DISPLAY1_NWR
	sh	t3,0(t1)
	/* set WR~ high */
	or	t3,DISPLAY1_NWR
	sh	t3,0(t1)
	/* set CE1~ & CE2~ high */
	or	t3,(DISPLAY1_NCE1 | DISPLAY1_NCE2)
	sh	t3,0(t1)
	/* next char */
	srl	a0,8
	addu	t3,DISPLAY1_A0
	bnez	a0,88b
	j	ra
END(sbddisplay)	
	
	
	.lcomm	wbfltmp,4

LEAF(wbflush)
XLEAF(mips_wbflush)
	la	t0,wbfltmp
	or	t0,K1BASE
	sync
	lw	zero,0(t0)
	j	ra
END(wbflush)

/* N microsecond delay */
LEAF(sbddelay)
	li	t1,CACHEUS(1)
	and	t0,ra,0x20000000
	beqz	t0,1f
	li	t1,ROMUS(1)
1:	mul	a0,t1
	subu	a0,15		# approx number of loops so far

	.set	noreorder	
	.set	nomacro
	nop
2:	bgtz	a0,2b
	subu	a0,1
	.set	macro
	.set	reorder

	j	ra
END(sbddelay)


LEAF(mips_cycle)
	.set	noreorder	
	.set	nomacro
1:	bgtz	a0,1b
	subu	a0,1
	.set	macro
	.set	reorder
	j	ra
END(mips_cycle)

