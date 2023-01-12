/* $Id: sbdreset.S,v 1.8 1998/07/23 23:35:34 chris Exp $ */
/*
** Copyright 1997 Algorithmics Ltd
**	All Rights Reserved
**	
** gal9/sbdreset.sx -- low level board dependent routines
*/

#include <mips.h>
#include <pmon.h>
#include <sbd.h>

#include "gt64011.h"
#include "z8530.h"
			
#include "meminit.s"

#define MB	0x100000
				
#define MemTypeNone		0x8000
#define MemRasMask		0x0f00
#define MemRasShift		8
#define MemCasMask		0x000f
#define MemCasShift 		0

#define rasave	s0
#define p64011	s1
#define bank0	s2
#define bank1	s3
#define bank2	s4
#define bank3	s5
#define memtop	s6
#define membase	s7
	
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
				
#undef DBGSBD
	
#ifdef DBGSBD
#define DBG(s) \
	.rdata ; \
88:	.asciiz	s ; \
	.text ; \
	la	a0, 88b ; \
	jal	_dbgmsg
	
LEAF(_dbgmsg)
	.set noat
	li	AT,PHYS_TO_K1(Z8530_CHANB)
waitrdy:
	lbu	v1,CMD(AT)
	.set noreorder;	nop; nop; nop; nop; nop; nop; nop; nop; .set reorder
	and	v1,US_TXRDY
	beqz	v1,waitrdy
	
	lbu	v1,(a0)
	addu	a0,1
	beqz	v1,9f
	sb	v1,DATA(AT)
	.set noreorder;	nop; nop; nop; nop; nop; nop; nop; nop; .set reorder
	b	waitrdy	
9:	j	ra
	.set at
END(_dbgmsg)

LEAF(_dbghex)
	li	a1,PHYS_TO_K1(Z8530_CHANB)
	li	t0,8
1:
	lbu	t1,CMD(a1)
	.set noreorder;	nop; nop; nop; nop; nop; nop; nop; nop; .set reorder
	and	t1,US_TXRDY
	beqz	t1,1b
	
	srl	t1,a0,28
	addu	t1,'0'
	ble	t1,'9',2f
	addu	t1,'a'-'0'-10
2:	sb	t1,DATA(a1)
	.set noreorder;	nop; nop; nop; nop; nop; nop; nop; nop; .set reorder

	sll	a0,4
	sub	t0,1
	bnez	t0,1b
		
	j	ra
	.set at
END(_dbghex)
	
	.rdata
initb_str:
	.byte	9,0x40	/* Reset CH B */
	.byte	1,0x00	/* Interrupt disabled */
	.byte	3,0xc1	/* 8 bits/char rx enable */
	.byte	4,0x44	/* x16 clk mode 1 stop bit */
	.byte	5,0x6a	/* tx 8/bit RTS & tx enable */
	.byte	9,0x0a	/* MIE Master int enab. and NV No Vector */
	.byte	11,0x50	/* Select BR gen. out for both rx and ts */
	.byte	0,0x10
	.byte	0,0x10
	.byte	14,0x01	/* enable baud rate gen. */
	.byte	15,0x00	/* known state for reg 15 */
	
	.byte	14,0x00	/* disable baud rate gen. */
	.byte	12,0x0a	/* 9600 baud time const. - lower 8 bits */
	.byte	13,0x00	/* 9600 buad time const. - upper 8 bits */
	.byte	14,0x01	/* enable baud rate gen. */
	.byte	0xff

	.text
	
SLEAF(_dbginit)
	li	v0,PHYS_TO_K1(Z8530_CHANB)
	la	a0,initb_str
	or	a0,K1BASE
1:	lbu	t0,0(a0)
	beq	t0,0xff,1f
	sb	t0,CMD(v0)
	.set noreorder;	nop; nop; nop; nop; nop; nop; nop; nop; .set reorder
	addu	a0,1
	b	1b
1:	j	ra
SEND(_dbginit)		
#else
#define DBG(s)		
#endif
		
LEAF(sbdreset)
	move	rasave,ra

	/* if launched by ITROM, leave Config alone */
#ifndef ITBASE	
	/* set config register for 32b/32b cachelines, kseg0 cacheable */
	mfc0	t1,C0_CONFIG
	and	t1,~0x3f		# set bits 5..0 only
	or	t1,CFG_IB | CFG_DB | CFG_C_NONCOHERENT
	mtc0	t1,C0_CONFIG
#endif	

	/*
	 * slight amount of kludgery here to stop RAM resident
	 * program from overwriting itself...
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
	b	1b

8:

	
#ifdef DBGSBD
	jal	_dbginit
	DBG("sbdreset\r\n")
#endif
			
	li	p64011, PA_TO_KVA1(GT64011_BASE)
	
	/*
	 * Fix up the FLASH/EPROM configuration
	 * If the Boot device is width 8 then 
	 * set bank1 to width 64 to enable the flash
	 * otherwise set to width 8 to enable the ROM
	 */
	lw	t0,GT_DEVPAR_BOOT(p64011)
	and	t0,htoll(GT_DEVPAR_DevWidthMASK)
	lw      t1,GT_DEVPAR_BANK1(p64011) 
	and     t1,htoll(~GT_DEVPAR_DevWidthMASK) 
	beq	t0,htoll(GT_DEVPAR_DevWidth8),.bootedfromprom
.bootedfromflash:
	/* set bank1 (prom) to 8 bit device */
	or	t1,htoll(GT_DEVPAR_DevWidth8)
	b	1f
.bootedfromprom:
	/* set bank1 (flash) to 64 bit device */
	or	t1,htoll(GT_DEVPAR_DevWidth64)
	/* b	1f */
1:	
	sw      t1,GT_DEVPAR_BANK1(p64011) 
	
			
	/* configure DRAM
 	 * 
	 * each bank can be configured as 16Mx32 interleaved (128Mb total)
	 * Due to address space limitations we map each bank in turn to
	 * determine whether the slot is populated and what size RAMS are
	 * used.
         */

	/* set bank parameters */
	
#define GT_DRAMPAR_init \
			GT_DRAMPAR_CASWr2 | \
			GT_DRAMPAR_RAStoCASWr3 | \
			GT_DRAMPAR_CASRd2 | \
			GT_DRAMPAR_RAStoCASRd3 | \
			GT_DRAMPAR_ParityDisable
	
	DBG("Initialising DRAMPAR registers\r\n");
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
	li	t1, htoll(((DRAM_BASE + 128*MB - 1) >> 20) & GT_DDAS_HIMASK_High);
	sw	t0, GT_DDAS_RAS0LO(p64011)
	sw	t1, GT_DDAS_RAS0HI(p64011)
		
	DBG("bank0 memType\r\n");
	bal	memType
	move	bank0, v0

	/* disable bank 0, enable bank1 */
	li	t0, htoll(GT_DDAS_LOMASK_Low)
	li	t1, htoll(0);
	sw	t0, GT_DDAS_RAS0LO(p64011)
	sw	t1, GT_DDAS_RAS0HI(p64011)
	li	t0, htoll(((DRAM_BASE +   0*MB) >> 20) & GT_DDAS_LOMASK_Low)
	li	t1, htoll(((DRAM_BASE + 128*MB - 1) >> 20) & GT_DDAS_HIMASK_High);
	sw	t0, GT_DDAS_RAS1LO(p64011)
	sw	t1, GT_DDAS_RAS1HI(p64011)
		
	DBG("bank1 memType\r\n");
	bal	memType
	move	bank1, v0

	/* disable bank 1 */
	li	t0, htoll(GT_DDAS_LOMASK_Low)
	li	t1, htoll(0);
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
	
	DBG("bank2 memType\r\n");
	bal	memType
	move	bank2, v0

	/* disable bank 2, enable bank3 */
	li	t0, htoll(GT_DDAS_LOMASK_Low)
	li	t1, htoll(0);
	sw	t0, GT_DDAS_RAS2LO(p64011)
	sw	t1, GT_DDAS_RAS2HI(p64011)
	li	t0, htoll(((DRAM_BASE +   0*MB) >> 20) & GT_DDAS_LOMASK_Low)
	li	t1, htoll(((DRAM_BASE + 128*MB - 1) >> 20) & GT_DDAS_HIMASK_High);
	sw	t0, GT_DDAS_RAS3LO(p64011)
	sw	t1, GT_DDAS_RAS3HI(p64011)
		
	DBG("bank3 memType\r\n");
	bal	memType
	move	bank3, v0

	/* disable bank 3 */
	li	t0, htoll(GT_DDAS_LOMASK_Low)
	li	t1, htoll(0);
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
	
	DBG("bank0 memParm\r\n");
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
	
	
1:	DBG("bank1 memParm\r\n");
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


	DBG("configuring ras0:1\r\n");
	
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
	
1:	DBG("bank2 memParm\r\n");
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
	
1:	DBG("bank3 memParm\r\n");
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

	DBG("configuring ras3:2\r\n");
	
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
#define DREG(str,rname) \
	DBG(str); \
	DBG(":\t") ;			\
	lw	a0,rname(p64011) ;	\
	HTOLL(a0,t0) ;			\
	jal	_dbghex ;		\
	DBG("\r\n")
	
	DBG("This is how I see it...\r\n")
	DREG("DRAMPAR_BANK0",GT_DRAMPAR_BANK0)
	DREG("DRAMPAR_BANK1",GT_DRAMPAR_BANK1)
	DREG("DRAMPAR_BANK2",GT_DRAMPAR_BANK2)
	DREG("DRAMPAR_BANK3",GT_DRAMPAR_BANK3)
	DREG("PAS_RAS10LO",GT_PAS_RAS10LO)
	DREG("PAS_RAS10HI",GT_PAS_RAS10HI)
	DREG("PAS_RAS32LO",GT_PAS_RAS32LO)
	DREG("PAS_RAS32HI",GT_PAS_RAS32HI)
	DREG("DDAS_RAS0LO",GT_DDAS_RAS0LO)
	DREG("DDAS_RAS0HI",GT_DDAS_RAS0HI)
	DREG("DDAS_RAS1LO",GT_DDAS_RAS1LO)
	DREG("DDAS_RAS1HI",GT_DDAS_RAS1HI)
	DREG("DDAS_RAS2LO",GT_DDAS_RAS2LO)
	DREG("DDAS_RAS2HI",GT_DDAS_RAS2HI)
	DREG("DDAS_RAS3LO",GT_DDAS_RAS3LO)
	DREG("DDAS_RAS3HI",GT_DDAS_RAS3HI)

#define DBANK(str,reg) \
	DBG(str) ; \
	DBG(":\t"); \
	move a0,reg ; \
	jal	_dbghex ; \
	DBG("\r\n")
	DBANK("ras0",bank0)
	DBANK("ras1",bank1)
	DBANK("ras2",bank2)
	DBANK("ras3",bank3)
	
#if 0
	/* now undo all that hard work! */

		
	li	t0,0x5ffc6f14
	sw	t0,GT_DRAMPAR_BANK0(p64011)
	sw	t0,GT_DRAMPAR_BANK1(p64011)
	sw	t0,GT_DRAMPAR_BANK2(p64011)
	sw	t0,GT_DRAMPAR_BANK3(p64011)
	
	li	t0,0
	sw	t0,GT_PAS_RAS10LO(p64011)
	sw	t0,GT_PAS_RAS10HI(p64011)

	li	t0,0x07000000
	sw	t0,GT_PAS_RAS32LO(p64011)
	sw	t0,GT_PAS_RAS32HI(p64011)
		
	li	t0,0x00000000
	sw	t0,GT_DDAS_RAS0LO(p64011)
	li	t0,0x07000000
	sw	t0,GT_DDAS_RAS0HI(p64011)
	li	t0,0x08000000
	sw	t0,GT_DDAS_RAS1LO(p64011)
	li	t0,0x0f000000
	sw	t0,GT_DDAS_RAS1HI(p64011)
	li	t0,0x10000000
	sw	t0,GT_DDAS_RAS2LO(p64011)
	li	t0,0x17000000
	sw	t0,GT_DDAS_RAS2HI(p64011)
	li	t0,0x18000000
	sw	t0,GT_DDAS_RAS3LO(p64011)
	li	t0,0x1f000000
	sw	t0,GT_DDAS_RAS3HI(p64011)
#endif
		
	DBG("Stashing bank values\r\n")
	li	a0,PA_TO_KVA1(0x1fff0)
	sw	bank0,0(a0)
	sw	bank1,4(a0)
	sw	bank2,8(a0)
	sw	bank3,12(a0)
#endif
		
	/* we can now initialise the caches for a fast clear_mem */
	SBD_DISPLAY ('C','A','C','H',CHKPNT_CACH)
	DBG("init_cache\r\n")
	jal	mips_init_cache

.noinit:

	/* initialise tlb */
	SBD_DISPLAY ('I','T','L','B', CHKPNT_ITLB)
	DBG("init_tlb\r\n")
	bal	init_tlb

	DBG("sbdreset completed\r\n")
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
#define GT_DRAM_CFG_INIT \
	GT_DRAM_CFG_RefIntCnt(160) | \
	GT_DRAM_CFG_StagRefOn | \
	GT_DRAM_CFG_ADSFunctDRAM | \
	GT_DRAM_CFG_DRAMLatchActive
	
/* serial port:	 widest timings even 8 bit bus, latch enabled no parity */
#define GT_DEVPAR_SERIALINIT \
	GT_DEVPAR_TurnOff(7) | \
	GT_DEVPAR_AccToFirst(15) | \
	GT_DEVPAR_AccToNext(15) | \
	GT_DEVPAR_ADStoWr(7) | \
	GT_DEVPAR_WrActive(7) | \
	GT_DEVPAR_WrHigh(7) | \
	GT_DEVPAR_DevWidth8 | \
	GT_DEVPAR_DevLocEven | \
	GT_DEVPAR_LatchFunctTransparent | \
	GT_DEVPAR_ParityDisable | \
	GT_DEVPAR_Reserved

/* PCI:	*/	
#define GT_IPCI_TOR_INIT \
	GT_IPCI_TOR_Timeout0(255) | \
	GT_IPCI_TOR_Timeout1(255) | \
	GT_IPCI_TOR_RetryCtr(0)

#define INIT(addr,val) \
	.word	addr, val
#define GTINIT(addr,val) \
	INIT(PHYS_TO_K1(GT64011_BASE+(addr)), htoll(val))
	
	.rdata
reginittab:

	/* disable ras1:0 and ras3:2 decodes */
	GTINIT(GT_PAS_RAS10LO,	GT_PAS_LOMASK_Low);
	GTINIT(GT_PAS_RAS10HI,	0);
	GTINIT(GT_PAS_RAS32LO,	GT_PAS_LOMASK_Low);
	GTINIT(GT_PAS_RAS32HI,	0);
	
	/* disable RAS[0123] */
	GTINIT(GT_DDAS_RAS0LO,	GT_DDAS_LOMASK_Low)
	GTINIT(GT_DDAS_RAS0HI,	0);
	GTINIT(GT_DDAS_RAS1LO,	GT_DDAS_LOMASK_Low)
	GTINIT(GT_DDAS_RAS1HI,	0);
	GTINIT(GT_DDAS_RAS2LO,	GT_DDAS_LOMASK_Low)
	GTINIT(GT_DDAS_RAS2HI,	0);
	GTINIT(GT_DDAS_RAS3LO,	GT_DDAS_LOMASK_Low)
	GTINIT(GT_DDAS_RAS3HI,	0);
	
	GTINIT(GT_DRAM_CFG,	GT_DRAM_CFG_INIT)
	GTINIT(GT_DEVPAR_BANK0, GT_DEVPAR_SERIALINIT)
	GTINIT(GT_IPCI_TOR,	GT_IPCI_TOR_INIT)
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

	.lcomm	wbfltmp,4

LEAF(wbflush)
XLEAF(mips_wbflush)
	sync
	la	t0,wbfltmp
	or	t0,K1BASE
	lw	zero,0(t0)
	j	ra
END(wbflush)


LEAF(sbddelay)
	li	t1,CACHEUS
	and	t0,ra,0x20000000
	beqz	t0,1f
	li	t1,ROMUS
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

