/*
 * p6032/sbdreset.sx: low level reset code for P-6032/P-6064
 *
 * Copyright (c) 2000 Algorithmics Ltd - all rights reserved.
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the "Free MIPS" License Agreement, a copy of 
 * which is available at:
 *
 *  http://www.algor.co.uk/ftp/pub/doc/freemips-license.txt
 *
 * You may not, however, modify or remove any part of this copyright 
 * message if this program is redistributed or reused in whole or in
 * part.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * "Free MIPS" License for more details.  
 */

#define	P60XX_I2C
#define	P60XX_PARITY
		
#if !#cpu(r4000) && !#cpu(r5000)
# error Wrong CPU type selected
#endif

#if __mips < 3
# error Must be compiled -mips3 or greater
#endif
	
#ifdef IN_PMON	
#include <mips.h>
#include <pmon.h>
#include <sbd.h>
#define _sbd_reset	sbdreset
#else	
#include <mips/asm.h>
#include <mips/regdef.h>
#include <mips/cpu.h>
#include <mips/endian.h>
#include "sbd.h"
#endif
#include "hd2532.h"
#include "i82371eb.h"
#include "i8254.h"
#include "pc97307.h"
#include "isapnpreg.h"
#include "pcireg.h"
		
#include <mips/prid.h>
	
#if !__mips64	
	/* force 64-bit register support */
	.set	gp64
#endif
	
/* CPU-specific bootmode bitstreams */	
#if #endian(big)	
#define BOOTMODE_RM7000		0x02034100
#define BOOTMODE_RM52XX		0x00230100
#define BOOTMODE_R4650		0x00001100
#define BOOTMODE_RC6447X	0x00001100
#define BOOTMODE_RC6457X	0x06001100
#define BOOTMODE_R5500		0x00000100
#else
#define BOOTMODE_RM7000		0x02034000
#define BOOTMODE_RM52XX		0x00230000
#define BOOTMODE_R4650		0x00001000
#define BOOTMODE_RC6447X	0x00001000
#define BOOTMODE_RC6457X 	0x06001000
#define BOOTMODE_R5500		0x00000000
#endif
	
#define WBFLUSH 

#define MEG	0x100000		
	
#define CFGADDR(idsel,function,reg) ((1<<(11+(idsel)))+((function)<<8)+(reg))
	
#define DISPLAY(d0,d1,d2,d3,d4,d5,d6,d7) \
	li	t8,PA_TO_KVA1(LED_BASE+HD2532_CRAM);	\
	li	t9,d0;				\
	sw	t9,HD2532_CHAR(0)(t8); 		\
	li	t9,d1; 				\
	sw	t9,HD2532_CHAR(1)(t8);		\
	li	t9,d2;	 			\
	sw	t9,HD2532_CHAR(2)(t8);		\
	li	t9,d3;	 			\
	sw	t9,HD2532_CHAR(3)(t8);		\
	li	t9,d4;				\
	sw	t9,HD2532_CHAR(4)(t8); 		\
	li	t9,d5; 				\
	sw	t9,HD2532_CHAR(5)(t8);		\
	li	t9,d6;	 			\
	sw	t9,HD2532_CHAR(6)(t8);		\
	li	t9,d7;	 			\
	sw	t9,HD2532_CHAR(7)(t8)

	
#define DBGSBD 2
#undef DBGSBD

#if !defined(DBGSBD)
#define DBGDISPLAY(d0,d1,d2,d3,d4,d5,d6,d7)
#define DBGLA(reg, label)
#define DBGLW(reg, label)
#define DBGSTRING(label, string)
#define DBGTSTRING(string)
#define DBGLSTR(reg, string)
#define DBGPSTR(string)
#else	
#define DBGDISPLAY(d0,d1,d2,d3,d4,d5,d6,d7) \
	DISPLAY(d0,d1,d2,d3,d4,d5,d6,d7); \
	li	t8,ROMMS(100); \
	.set	noreorder; \
99:	bnez	t8,99b; \
	subu	t8,1; \
	.set	reorder
#define DBGLA(reg, label)	 \
	LA(reg, label);		 \
	or	reg,KSEG1_BASE
		
#define DBGLW(reg, label)	\
	DBGLA	reg,label;	\
	lw	reg,0(reg)

#define DBGSTRING(label, string)	\
	.rdata;			\
label:	.asciiz string;		\
	.previous
	
#define DBGTSTRING(string)		\
	.rdata;			\
	.word	9f;		\
	.section .rodata1; 	\
9:	.asciiz string

#define DBGLSTR(reg, string)	\
	DBGLA(reg,9f);		\
	DBGSTRING(9,string)

/* print a string */
#define DBGPSTR(string)	\
	DBGLSTR(a0,string);		\
	bal	rom_prs
	
#include "ns16550.h"
	
#define TXWAIT		10000
	
#ifndef NS16550_INB
#define NS16550_INB(dst,offs,base) \
	lbu	dst,offs(base)
#endif
	
#ifndef NS16550_OUTB
#define NS16550_OUTB(src,offs,base) \
	sb	src,offs(base)
#endif
	
	
/*
 * rom_outch (int c)
 * Print a character on console device
 */
SLEAF(rom_outch)
	/* blocking transmit, with timeout */
	li	t1,PA_TO_KVA1(UART0_BASE)
	bne	a0,'\n',2f
	li	a0,'\r'
2:			
	li	t0,TXWAIT			# timeout
1:	NS16550_INB(v0,LSR,t1)			# get LSR
	and	v0,LSR_TXRDY			# tx ready?
	bnez	v0,1f				# yup - go and write
	subu	t0,1				# continue until timeout
	bnez	t0,1b
1:	NS16550_OUTB(a0,DATA,t1)		# write data
	bne	a0,'\r',1f
	li	a0,'\n'
	b	2b
1:		
	j	ra
SEND(rom_outch)
	

/*
 * rom_prnl ()
 * Print newline on console device
 */
LEAF(rom_prnl)
	move	t7,ra
	li	a0,'\n'
	bal	rom_outch
	j	t7
END(rom_prnl)


/*
 * rom_prs (char *s)
 * Print a string on console device
 */
LEAF(rom_prs)
	move	t7,ra
	move	t6,a0

1:	lbu	a0,0(t6)
	beqz	a0,2f
	bal	rom_outch
	addu	t6,1
	b	1b

2:	j	t7
END(rom_prs)


/*
 * rom_prx (unsigned int x, unsigned int log2size)
 * Print a register on console device as hex digits
 */
LEAF(rom_prx)
	move	t7,ra
	
	li	t5,8			# t5 = bitcnt = 8 << log2size
	sll	t5,a1			
	
#if __mips64
	li	t6,64			# t6 = regsize - bitcnt
	subu	t6,t5
	dsll	t6,a0,t6		# get sig part of a0 to top of t6
1:	dsrl	a0,t6,60		# get top 4 bits
	addu	a0,'0'
	ble	a0,'9',2f
	addu	a0,'A'-'0'-10
2:	bal	rom_outch
	dsll	t6,4
	subu	t5,4
	bnez	t5,1b
#else
	li	t6,32			# t6 = regsize - bitcnt
	subu	t6,t5
	sll	t6,a0,t6		# get sig part of a0 to top of t6
1:	srl	a0,t6,28		# get top 4 bits
	addu	a0,'0'
	ble	a0,'9',2f
	addu	a0,'A'-'0'-10
2:	bal	rom_outch
	sll	t6,4
	subu	t5,4
	bnez	t5,1b
#endif	
	
	j	t7
END(rom_prx)

SLEAF(_rom_consinit)
	li	v0,PA_TO_KVA1(UART0_BASE)
	# enable 16550 fifo if it is there
	li	t0,FIFO_ENABLE|FIFO_RCV_RST|FIFO_XMT_RST|FIFO_TRIGGER_4
	NS16550_OUTB(t0,FIFO,v0)
	
	# 100us delay 
	.set	noreorder
	li	t0,CACHEUS(100)
1:	bnez	t0,1b
	subu	t0,1
	.set	reorder
	
	li	a1,9600

	/* wait for all tx data to be sent */
	li	t0,TXWAIT
1:	NS16550_INB(t1,LSR,v0)
	and	t1,LSR_TSRE
	bnez	t1,2f
	subu	t0,1
	bnez	t0,1b
	
	/* convert baud rate in a1 into register value */
2:	beqz	a1,.fail			# don't want divide error
	li	t2,NS16550_HZ/16		# brtc = CLK/16/speed
	divu	t2,a1
	blez	t2,.fail			# brtc must be > 0
	
	li	t0,CFCR_DLAB			# select brtc divisor
	NS16550_OUTB(t0,CFCR,v0)			
	NS16550_OUTB(t2,DLL,v0)			# store divisor lsb
	srl	t2,8	
	NS16550_OUTB(t2,DLM,v0)			# store divisor msb
	li	t0,CFCR_8BITS			# set 8N1 mode
	NS16550_OUTB(t0,CFCR,v0)
	
	li	t0,MCR_DTR|MCR_RTS|MCR_IENABLE	# enable DTR & RTS
	NS16550_OUTB(t0,MCR,v0)
	
	li	t0,IER_ERXRDY			# enable receive interrupt(!)
	NS16550_OUTB(t0,IER,v0)
	
	move	v0,zero				# indicate success
	j	ra
	
.fail:	li	v0,1
	j	ra
	
SEND(_rom_consinit)			
#endif
	
/*
 * Basic board initialisation, called straight from RESET.
 * It is only called if this program is built for ROM.  
 * It can use any registers except s8, k0 and k1.
 *
 * Note that s8 holds a "relocation factor" (see ../share/romlow.sx)
 * which must be added to any address before it is used.  This
 * is to support relocatable roms.
 */
	
#define	RELOC		s8
#define tmpsize		s0
#define msize		s1
#define sdShape		s2
#define bonito		s3
#define dbg		s4
#define sdCfg		s5
#define	rasave		s7

#define LA(r,a)	\
	la	r,a; \
	addu	r,RELOC
		
LEAF(_sbd_reset)
	move	rasave,ra
	
#if DBGSBD > 0
	/* Initialise the LED so we can write to it */
	li	a1, PA_TO_KVA1(LED_BASE+HD2532_CW)
	li	a2,HD2532_CW_C
	sw	a2,(a1)

	.set	noreorder
	li	t0,CACHEUS(110)
1:	bnez	t0,1b
	subu	t0,1
	.set	reorder

	DBGDISPLAY('D','B','G','S','B','D',' ',' ');
#endif
	
#if defined(IN_PMON)	
	move	RELOC,zero
#endif	
	mfc0	t2,C0_PRID		# get PrID
	mtc0	zero,$18		# C0_IWATCH/C0_WATCHLO
	mtc0	zero,$19		# C0_DWATCH/C0_WATCHHI

	/* get global pointer to Bonito registers */
	li	bonito,PA_TO_KVA1(BONITO_BASE)
	
	srl	t2,8
	and	t2,0xff

	/*
	 * some CPUs need a non-zero bootmode stream
	 * we achieve this by loading the bitstream into the intpol
	 * register and issuing a self-reset
	 * but first we must avoid getting into an infinite loop...
	 */

	lw	t3,BONITO_BONGENCFG(bonito)
	and	t3,BONITO_BONGENCFG_CPUSELFRESET
	bnez	t3,.noreset

#ifdef P6032
	/*
	 * if the bootmode stream is from the
	 * CPLD we will assume it is correct
	 */
	lw	t0,BONITO_BONPONCFG(bonito)
	and	t0,BONITO_BONPONCFG_CPUTYPE
	beq	t0,(0x4 << BONITO_BONPONCFG_CPUTYPE_SHIFT),.noreset

#ifdef BOOTMODE_RM52XX
1:	bne	t2,PRID_RM52XX,1f
	li	t3,BOOTMODE_RM52XX
	b	.reset
#endif	
#ifdef BOOTMODE_RC6447X
1:	bne	t2,PRID_RC6447X,1f
	li	t3,BOOTMODE_RC6447X
	b	.reset
#endif
#ifdef BOOTMODE_RC6457X
1:	bne	t2,PRID_RC6457X,1f
	li	t3,BOOTMODE_RC6457X
	b	.reset
#endif
#ifdef BOOTMODE_R4650
1:	bne	t2,PRID_R4650,1f
	li	t3,BOOTMODE_R4650
	b	.reset
#endif
1:	b	.noreset
	
.reset:	
	DBGDISPLAY('R','e','s','e','t','R','e','q')
	
	# use the switch settings to modify the base settings 
	li	t5,PA_TO_KVA1(CPLD_BASE)
	lhu	t0,CPLD_REVISION(t5)
	lhu	t4,CPLD_SWITCHES(t5)
	bgeu	t0,2,1f
	lhu	t4,CPLD_SWITCHES_PREREV2(t5)
1:	
	# COPT[1] -> bootmode[20]
	and	t5,t4,CPLD_SW_CPUOPT & (0x02<<CPLD_SW_CPUOPT_SHIFT)
	sll	t5,20-(CPLD_SW_CPUOPT_SHIFT+1)
	or	t3,t5
	bnez	t5,1f
	
	# CDIV -> bootmode[6:5]
	and	t5,t4,CPLD_SW_CPUDIV
	sll	t5,5-CPLD_SW_CPUDIV_SHIFT
	or	t3,t5
	b	2f
	
1:
	# CDIV [3:2] -> bootmode[7:6]
	and	t5,t4,CPLD_SW_CPUDIV
	sll	t5,6-CPLD_SW_CPUDIV_SHIFT
	or	t3,t5
	# 1 -> bootmode[5]
	or	t3,1<<5
2:	
	
#if 0	/* we know what endianess we are/want */
	# BIGENDIAN -> bootmode[8]
	and	t5,t4,CPLD_SW_BIGEND
	sll	t5,8-CPLD_SW_BIGEND_SHIFT
	or	t3,t5
#endif		
#endif
#ifdef P6064
	
	.rdata
	.align	2
#define CDIV(mb20,mb7_5) \
	.word	0+((mb20<<20)|(mb7_5<<5))
		
cdiv_qed:
	CDIV(0,0)	# x2
	CDIV(1,3)	# x2.5
	CDIV(0,1)	# x3
	CDIV(1,5)	# x3.5
	CDIV(0,2)	# x4
	CDIV(1,7)	# x4.5
	CDIV(0,3)	# x5
	CDIV(0,3)	# x5
cdiv_5500:
	CDIV(0,0)	# x2
	CDIV(1,3)	# x2.5
	CDIV(0,1)	# x3
	CDIV(1,5)	# x3.5
	CDIV(0,2)	# x4
	CDIV(1,7)	# x4.5
	CDIV(0,3)	# x5
	CDIV(1,1)	# x5.5
	
	.struct 0
#define CPU(prid,bootmode,cdiv,xtra) \
	.word	prid,bootmode,cdiv,xtra
CPU_PRID:	.word	0
CPU_BOOTMODE:	.word	0
CPU_CDIV:	.word	0
CPU_XTRA:	.word	0
CPU_SIZE:	
	.previous
cpu_data:
	CPU(PRID_RM7000,BOOTMODE_RM7000,cdiv_qed,xtra_rm7000)
	CPU(PRID_RM52XX,BOOTMODE_RM52XX,cdiv_qed,0)
	CPU(PRID_RC6447X,BOOTMODE_RC6447X,0,0)
	CPU(PRID_RC6457X,BOOTMODE_RC6457X,0,0)
	CPU(PRID_R4650,BOOTMODE_R4650,0,0)
	CPU(PRID_R5500,BOOTMODE_R5500,cdiv_5500,0)
	.word	0
	.text

	DBGDISPLAY('R','e','s','e','t','C','h','k')
	
	LA(t1,cpu_data)
	or	t1,KSEG1_BASE
	
1:	lw	t3,CPU_PRID(t1)
	beqz	t3,.noreset		# off end of table
	beq	t3,t2,.reset
	addu	t1,CPU_SIZE
	b	1b
		
.reset:	
	DBGDISPLAY('R','e','s','e','t','R','e','q')
	# use the switch settings to modify the base settings 

	# to access the switches we need to enable CS0 buffbit
	li	t4,BONITO_IODEVCFG_BUFFBIT_CS0
	sw	t4,BONITO_IODEVCFG(bonito)
	lw	zero,BONITO_IODEVCFG(bonito)

	li	t4,PA_TO_KVA1(CPLD_BASE)
	lw	t4,CPLD_SWITCHES(t4)

	lw	t3,CPU_BOOTMODE(t1)	# base setting
	lw	t5,CPU_CDIV(t1)
	beqz	t5,1f			# skip CDIV settings if we don't know how

	# get clock multiplier switches
	addu	t5,RELOC
	or	t5,KSEG1_BASE
	
	and	v0,t4,CPLD_CLKMULT
	srl	v1,v0,CPLD_CLKMULT_SHIFT
	sll	v1,2
	addu	t5,v1
	lw	v0,0(t5)
	
	or	t3,v0			# set multiplier bits

1:	/* others... */		
	lw	t5,CPU_XTRA(t1)
	beqz	t5,.doreset
	addu	t5,RELOC
	or	t5,KSEG1_BASE
	j	t5

xtra_rm7000:
	# set scache 
	b	.doreset
#endif		
	
		
.doreset:
	DBGDISPLAY('R','e','s','e','t','G','o',' ')
	sw	t3,BONITO_INTPOL(bonito)
	lw	t3,BONITO_INTPOL(bonito)
	lw	t3,BONITO_BONGENCFG(bonito)
	or	t3,BONITO_BONGENCFG_CPUSELFRESET
	sw	t3,BONITO_BONGENCFG(bonito)
	lw	t3,BONITO_BONGENCFG(bonito)

	DBGDISPLAY('R','e','s','e','t','.','.','.')
	
1:	b	1b		# loop forever
	
.noreset:
	DBGDISPLAY('N','o',' ','R','e','s','e','t')
	
	/* if we get here, the cpu has been correctly initialised */
	bne	t2,PRID_R4650,2f
	
	/* r4640/50 initialisation */
	mtc0	zero,$0			# C0_IBASE
	mtc0	zero,$1			# C0_IBOUND
	mtc0	zero,$2			# C0_DBASE
	mtc0	zero,$3			# C0_DBOUND
#ifndef BOOTPKG
	/* Making this dependent on BOOTPKG is wrong, but it is done this
	 * way to stop PMON reenabling the caches if ITROM has detected an
	 * error
	 */
	li	t3,0x22233333
	mtc0	t3,$17			# C0_CALG
#endif	
	b	3f
	
2:
	mfc0	t1,C0_CONFIG
	
	/* r4xxx/r5xxx initialisation */
	/* set config register for 32b/32b cachelines, kseg0 cacheable */
	and	t1,~0x3f		# set bits 5..0 only
	or	t1,CFG_IB | CFG_DB | CFG_C_WBACK
	
	/* Some processors have s/w programmable endianess */
#if #endian(big)
	or	t1,CFG_BE	
#else
	and	t1,~CFG_BE	
#endif
	/* set DDDD rate for CPUs that aren't hardware configured */
	and	t1,~CFG_EPMASK
	or	t1,CFG_EPD
	
	/* Vr5[45]00: set R4x00 compatible bus mode */
	beq	t2,PRID_R5400,0f
	beq	t2,PRID_R5500,0f
	b	2f
	
0:	and	t1,~CFG_EMMASK
	or	t1,CFG_EM_R4K
	b	2f
	
	/* Vr4100: switch off accelerated data (undefined on other CPUs)*/
1:	and	t1,~CFG_AD
2:	mtc0	t1,C0_CONFIG
	
3:	mfc0	t1,C0_STATUS		# get Status
	mtc0	zero,C0_CAUSE
	and	t1,SR_SR		# leave the SoftReset bit
	or	t1,SR_BEV		# force Boot Exception Vec
	mtc0	t1,C0_STATUS
	
	/* Initialise other low-level I/O devices */
	LA(a0,reginittab)
	or	a0,KSEG1_BASE
	bal	reginit

#if DBGSBD > 0
	
	li	a0,PA_TO_KVA1(PCI_IO_SPACE)
	li	a1,0x55
	li	a2,0xaa
	sb	a1,0x4d0(a0)
	sb	a2,0x4d1(a0)
	sb	a1,0x4d2(a0)
	sb	a2,0x4d3(a0)
	lbu	zero,0x4d0(a0)
	lbu	zero,0x4d1(a0)
	lbu	zero,0x4d2(a0)
	lbu	zero,0x4d3(a0)

	sb	a2,0x4d0(a0)
	sb	a1,0x4d1(a0)
	sb	a2,0x4d2(a0)
	sb	a1,0x4d3(a0)
	lbu	zero,0x4d0(a0)
	lbu	zero,0x4d1(a0)
	lbu	zero,0x4d2(a0)
	lbu	zero,0x4d3(a0)

	bal	_rom_consinit

	DBGPSTR("In _sbd_reset\n")

#if 1
	DBGPSTR("Raw read of PIIX config space: ");
	li	a0,CFGADDR(PCI_DEV_I82371,0,0)
	li	a1,PA_TO_KVA1(PCI_CFG_SPACE)
	and	a2,a0,0xffff
	or	a1,a2
	srl	a0,16
	li	a2,BONITO_BASE+BONITO_PCIMAP_CFG
	sw	a0,BONITO_PCIMAP_CFG(bonito)
	lw	zero,BONITO_PCIMAP_CFG(bonito)
	lw	a0,(a1)
	
	li	a1,2
	bal	rom_prx
	bal	rom_prnl	
	
	DBGPSTR("Raw halfword read of PIIX config space: ");
	li	a0,CFGADDR(PCI_DEV_I82371,0,0)
	li	a1,PA_TO_KVA1(PCI_CFG_SPACE)
	and	a2,a0,0xffff
	or	a1,a2
	srl	a0,16
	li	a2,BONITO_BASE+BONITO_PCIMAP_CFG
	sw	a0,BONITO_PCIMAP_CFG(bonito)
	lw	zero,BONITO_PCIMAP_CFG(bonito)
	lhu	a0,(a1)
	
	li	a1,2
	bal	rom_prx
	bal	rom_prnl	
	
	DBGPSTR("Raw word read of SMB base address: ");
	li	a0,CFGADDR(PCI_DEV_I82371,3,I82371_PCI3_SMBBA)
	li	a1,PA_TO_KVA1(PCI_CFG_SPACE)
	and	a2,a0,0xffff
	or	a1,a2
	srl	a0,16
	li	a2,BONITO_BASE+BONITO_PCIMAP_CFG
	sw	a0,BONITO_PCIMAP_CFG(bonito)
	lw	zero,BONITO_PCIMAP_CFG(bonito)
	lw	a0,(a1)
	
	li	a1,2
	bal	rom_prx
	bal	rom_prnl	
	
	DBGPSTR("Raw word read of SMB HSTCFG: ");
	li	a0,CFGADDR(PCI_DEV_I82371,3,I82371_PCI3_SMBHSTCFG) # byte???
	li	a1,PA_TO_KVA1(PCI_CFG_SPACE)
	and	a2,a0,0xffff
	or	a1,a2
	srl	a0,16
	li	a2,BONITO_BASE+BONITO_PCIMAP_CFG
	sw	a0,BONITO_PCIMAP_CFG(bonito)
	lw	zero,BONITO_PCIMAP_CFG(bonito)
	lhu	a0,(a1)
	
	li	a1,2
	bal	rom_prx
	bal	rom_prnl	
	
	DBGPSTR("Raw word read of SMB IO ENABLE: ");
	li	a0,CFGADDR(PCI_DEV_I82371,3,PCI_COMMAND_STATUS_REG)
	li	a1,PA_TO_KVA1(PCI_CFG_SPACE)
	and	a2,a0,0xffff
	or	a1,a2
	srl	a0,16
	li	a2,BONITO_BASE+BONITO_PCIMAP_CFG
	sw	a0,BONITO_PCIMAP_CFG(bonito)
	lw	zero,BONITO_PCIMAP_CFG(bonito)
	lw	a0,(a1)
	
	li	a1,2
	bal	rom_prx
	bal	rom_prnl	
	
#endif
		
#if DBGSBD > 1
	DBGPSTR("Resetting SMB\n")
	bal	i2creset
	
	DBGPSTR("SMB registers:\n")
	bal	i2cdump
	
	DBGPSTR("DIMM read\n")
	li	s0,0
loop:
	li	a0,0
	move	a1,s0
	bal	i2cread

	move	a0,v0
	li	a1,0
	bal	rom_prx

	li	a0,' '
	bal	rom_outch

	addu	s0,1
	and	t0,s0,15
	bnez	t0,1f

	bal	rom_prnl
1:	li	t0,128
	bltu	s0,t0,loop
#endif
#endif

#ifdef P60XX_I2C
	/* 
	 * Now determine DRAM configuration and size by
	 * reading the I2C EEROM on the DIMMS
	 */

	bal	i2creset
	li	msize,0
	
	DISPLAY ('D','I','M','M','0',' ',' ',' ')
	
	/* start with DIMM #0 */
	li	a0,0
	/* Do a RMW on SDCFG to preserve power up values */
	lw	sdCfg,BONITO_SDCFG(bonito)
	/* use parity if DIMMS support it */
	or	sdCfg,BONITO_SDCFG_DRAMPARITY
	
.nextdimm:
	li	sdShape,0
	
	/* read DIMM memory type (must be SDRAM) */
	li	a1,2
	bal	i2cread
	bne	v0,4,.nodimm
	
	/* read DIMM memory size per side */
	li	a1,31
	bal	i2cread
	beqz	v0,.nodimm
	sll	tmpsize,v0,22		# multiply by 4M
	
	/* read DIMM number of rows */
	li	a1,3
	bal	i2cread
	subu	v0,11
	bgtu	v0,14-11,.nodimm
	sll	v0,BONITO_SDCFG_AROWBITS_SHIFT
	and	v0,BONITO_SDCFG_AROWBITS
	or	sdShape,v0
	
2:	/* read DIMM number of cols */
	li	a1,4
	bal	i2cread
	subu	v0,8
	bgtu	v0,11-8,.nodimm
	sll	v0,BONITO_SDCFG_ACOLBITS_SHIFT
	and	v0,BONITO_SDCFG_ACOLBITS
	or	sdShape,v0
	
2:	/* read DIMM number of blocks-per-dram */
	li	a1,17
	bal	i2cread
	beq	v0,2,2f
	bne	v0,4,.nodimm
	or	sdShape,BONITO_SDCFG_ABANKBIT
	
2:	/* read DIMM number of sides (banks) */
	li	a1,5
	bal	i2cread
	beq	v0,1,2f
	bne	v0,2,.nodimm
	or	sdShape,BONITO_SDCFG_ASIDES
	sll	tmpsize,1	# msize *= 2	
	
2:	/* read DIMM width */
	li	a1,6
	bal	i2cread
	bleu	v0,36,2f
	bgtu	v0,72,.nodimm
	or	sdShape,BONITO_SDCFG_AWIDTH64

2:	/* check width for parity operation */
	beq	v0,36,2f
	beq	v0,72,2f

	and	sdCfg,~BONITO_SDCFG_DRAMPARITY
	
2:	/* check CAS latency */	

	/* always disable extra cycle for now */
	and	sdCfg,~BONITO_SDCFG_EXTRDDATA
	
2:	/* check for registered memory */
	li	a1,21
	bal	i2cread
	and	v0,0x12		/* registered DQMB, Addresss and control */
	bne	v0,0x12,2f

	/* Using registered DIMMs is a bit of a performance... */
	
	/* set the PIO bit */
	lw	v0,BONITO_GPIODATA(bonito)
	or	v0,PIO_REGENABLE
	sw	v0,BONITO_GPIODATA(bonito)

	/* set sdCfg.dramextregs -> 1 */
	or	sdCfg,BONITO_SDCFG_DRAMEXTREGS

	/* sdCfg.extrddata -> 0 */
	and	sdCfg,~BONITO_SDCFG_EXTRDDATA
	
	/* and we need toi do a config cycle... */	
	
2:
	addu	msize,tmpsize
	b	2f
		
.nodimm:
#if DBGSBD > 0
	move	dbg,a0
	DBGPSTR ("No DIMM in slot ")
	move	a0,dbg
	move	a1,zero
	bal	rom_prx
	bal	rom_prnl
	move	a0,dbg
#endif	
	or	sdShape,BONITO_SDCFG_AABSENT
	li	v0,0xff<<BONITO_SDCFG_AROWBITS_SHIFT
2:	beqz	a0,1f
	sll	sdShape,BONITO_SDCFG_BROWBITS_SHIFT
	li	v0,0xff<<BONITO_SDCFG_BROWBITS_SHIFT
1:	not	v0
	and	sdCfg,v0
	or	sdCfg,sdShape
	
	DISPLAY ('D','I','M','M','1',' ',' ',' ')
	addu	a0,1
	bltu	a0,2,.nextdimm

	/* If we are running in SDRAM, chop 4MB off the memory size,
	   and don't modify sdCfg register (assume someone in 
	   PCI-world has already set it up). */
	lw	t0,BONITO_BONPONCFG(bonito)
	and	t0,BONITO_BONPONCFG_ROMBOOT
	bne	t0,BONITO_BONPONCFG_ROMBOOT_SDRAM,1f
	
	beqz	msize,2f	# already zero!
	subu	msize,4*1024*1024
	b	2f
	
1:	sw	sdCfg,BONITO_SDCFG(bonito)
	
	/* Do a configuration cycle in case we changed something that needs one */
#ifndef BONITO_SDCFG_DRAMMODESET
#define BONITO_SDCFG_DRAMMODESET	(1<<21)
#define BONITO_SDCFG_DRAMMODESET_DONE	(1<<31)
#endif	
	or	t0,sdCfg,BONITO_SDCFG_DRAMMODESET
	sw	t0,BONITO_SDCFG(bonito)
1:	lw	t0,BONITO_SDCFG(bonito)
	and	t0,BONITO_SDCFG_DRAMMODESET_DONE
	beqz	t0,1b
	sw	sdCfg,BONITO_SDCFG(bonito)
	
2:		
#else	
	li	msize,4*0x100000
#endif
	
#if DBGSBD > 0
	DBGPSTR("sdcfg=");
	move	a0,sdCfg
	li	a1,2
	bal	rom_prx
	bal	rom_prnl
#endif
	
	/* XXX invalidate i/o buffer cache */
	
	li	t1,0		# accumulate pcimembasecfg settings
		
	/* set bar0 mask and translation to point to all memory */
	sub	t0,msize,1
	not	t0
	srl	t0,BONITO_PCIMEMBASECFG_ASHIFT-BONITO_PCIMEMBASECFG_MEMBASE0_MASK_SHIFT
	and	t0,BONITO_PCIMEMBASECFG_MEMBASE0_MASK
	or	t1,t0
	
	li	t0,0x00000000
	srl	t0,BONITO_PCIMEMBASECFG_ASHIFT-BONITO_PCIMEMBASECFG_MEMBASE0_TRANS_SHIFT
	and	t0,BONITO_PCIMEMBASECFG_MEMBASE0_TRANS
	or	t1,t0
	or	t1,BONITO_PCIMEMBASECFG_MEMBASE0_CACHED

	/* set bar1 to minimum size to conserve PCI space */
	li	t0, ~0
	srl	t0,BONITO_PCIMEMBASECFG_ASHIFT-BONITO_PCIMEMBASECFG_MEMBASE1_MASK_SHIFT
	and	t0,BONITO_PCIMEMBASECFG_MEMBASE1_MASK
	or	t1,t0
	
	li	t0,0x00000000
	srl	t0,BONITO_PCIMEMBASECFG_ASHIFT-BONITO_PCIMEMBASECFG_MEMBASE1_TRANS_SHIFT
	and	t0,BONITO_PCIMEMBASECFG_MEMBASE1_TRANS
	or	t1,t0
	or	t1,BONITO_PCIMEMBASECFG_MEMBASE1_CACHED

	sw	t1,BONITO_PCIMEMBASECFG(bonito)

	/* enable configuration cycles now */
	lw	t0,BONITO_BONPONCFG(bonito)
	and	t0,~BONITO_BONPONCFG_CONFIG_DIS
	sw	t0,BONITO_BONPONCFG(bonito)
			
#ifdef P60XX_PARITY	
	/* We have to clear memory to initialise parity */

	/* Skip memory clear if no memory */
	beqz	msize,.noclear
	
	/* Skip memory clear if non-parity memory */
	lw	t1,BONITO_SDCFG(bonito)
	and	t1,BONITO_SDCFG_DRAMPARITY
	beqz	t1,.noclear
	
	/* Skip memory clear if a soft reset */
	mfc0	t1,C0_STATUS
	and	t1,SR_SR
	bnez	t1,.noclear
	
	/* Clear bottom 256K running uncached */
	DISPLAY ('Z','2','5','6','K',' ',' ',' ')
	li	a0,PA_TO_KVA1(0)
	addu	a1,a0,256*1024
	.set noreorder
1:	sd	zero,0(a0)
	sd	zero,8(a0)
	sd	zero,16(a0)
	addu	a0,32
	bne	a0,a1,1b
	sd	zero,-8(a0)
	.set reorder
	
	/* We can now initialise the caches for a fast clear_mem */
	DISPLAY ('C','A','C','H',' ',' ',' ',' ')
#ifdef IN_PMON	
	jal	mips_init_cache	
#else	
	bal	mips_init_cache	
#endif
		
	/* Finally clear rest of memory running cached */
	li	a0,PA_TO_KVA1(256*1024)
	subu	a1,msize,256*1024
	blez	a1,.noclear
	addu	a1,a0
	
	/* clear all of memory (to set correct parity) */
	DISPLAY ('Z','M','E','M',' ',' ',' ',' ')
	
	/* run memory clear loop cached */
	LA(t0,1f)
	and	t0,~0x20000000
	j	t0
	
	.set noreorder
1:
	sd	zero,0(a0)
	sd	zero,8(a0)
	sd	zero,16(a0)
	sd	zero,24(a0)
	sd	zero,32(a0)
	sd	zero,40(a0)
	sd	zero,48(a0)
	addu	a0,64
	bne	a0,a1,1b
	sd	zero,-8(a0)
	.set reorder

	/* revert to uncached */
	LA(t0,.noclear)
	j	t0
.noclear:
		
#endif /* P60XX_PARITY */
		
	/* return to generic code, with available memory size */
	DISPLAY ('R','U','N',' ',' ',' ',' ',' ')
	move	ra,rasave
#ifdef IN_PMON		
	sw	msize,PA_TO_KVA1(0x180-4)
#else	
	move	v0,msize
#endif
	j	ra
END(_sbd_reset)


LEAF(_sbd_memfail)
	DISPLAY ('!','M','E','M','O','R','Y',' ')
1:	b	1b
	j	ra
END(_sbd_memfail)

#if #endian(big) && defined(P6032)
#define SMBOFFS(reg) (I82371_SMB_SMB/**/reg ^ 3)
#else
#define SMBOFFS(reg) I82371_SMB_SMB/**/reg
#endif
			

SLEAF(i2creset)
#if 0
	li	t0,PA_TO_KVA1(ISAPORT_BASE(SMB_PORT))
	li	t1,I82371_SMB_START|I82371_SMB_KILL
	sb	t1,SMBOFFS(HSTCMD)(t0)

1:	lbu	t1,SMBOFFS(HSTSTS)(t0)
	and	t2,t1,I82371_SMB_FAILED
	beqz	t2,1b

	sb	zero,SMBOFFS(HSTCMD)(t0)
	sb	t1,SMBOFFS(HSTSTS)(t0)
#endif
	j	ra
SEND(i2creset)
	

/* i2cread (unsigned dev, unsigned offs) */
SLEAF(i2cread)	
	li	t0,PA_TO_KVA1(ISAPORT_BASE(SMB_PORT))

	lbu	t1,SMBOFFS(HSTSTS)(t0)
	and	t1,~(I82371_SMB_FAILED|I82371_SMB_BUS_ERR|I82371_SMB_DEV_ERR|I82371_SMB_INTER)
	sb	t1,SMBOFFS(HSTSTS)(t0)
	
	sll	t1,a0,1
	or	t1,0xa1		# DIMM base address and read bit
	sb	t1,SMBOFFS(HSTADD)(t0)
	sb	a1,SMBOFFS(HSTCMD)(t0)
	
	
	li	t1,I82371_SMB_START|I82371_SMB_BDRW
	sb	t1,SMBOFFS(HSTCNT)(t0)

	li	t3,10000	
1:	lbu	t1,SMBOFFS(HSTSTS)(t0)
	and	t2,t1,I82371_SMB_FAILED|I82371_SMB_BUS_ERR|I82371_SMB_DEV_ERR|I82371_SMB_INTER
	bnez	t2,1f
	sub	t3,1
	bnez	t3,1b
	b	9f		
1:	
	# clear pending errors/interrupts
	sb	t1,SMBOFFS(HSTSTS)(t0)
	
	and	t2,t1,I82371_SMB_FAILED|I82371_SMB_BUS_ERR|I82371_SMB_DEV_ERR
	bnez	t2,9f

	lbu	v0,SMBOFFS(HSTDAT0)(t0)
	j	ra
	
9:	li	v0,-1
	j	ra
SEND(i2cread)	
	
#if DBGSBD > 1
SLEAF(i2cdump)
	move	t4,ra
	
	li	a0,PA_TO_KVA1(ISAPORT_BASE(SMB_PORT))
	lbu	a0,SMBOFFS(HSTSTS)(a0)
	li	a1,1
	bal	rom_prx
	bal	rom_prnl
		
	li	a0,PA_TO_KVA1(ISAPORT_BASE(SMB_PORT))
	lbu	a0,SMBOFFS(SLVSTS)(a0)
	li	a1,1
	bal	rom_prx
	bal	rom_prnl
	
	li	a0,PA_TO_KVA1(ISAPORT_BASE(SMB_PORT))
	lbu	a0,SMBOFFS(HSTCNT)(a0)
	li	a1,1
	bal	rom_prx
	bal	rom_prnl
	
	li	a0,PA_TO_KVA1(ISAPORT_BASE(SMB_PORT))
	lbu	a0,SMBOFFS(HSTCMD)(a0)
	li	a1,1
	bal	rom_prx
	bal	rom_prnl
	
	li	a0,PA_TO_KVA1(ISAPORT_BASE(SMB_PORT))
	lbu	a0,SMBOFFS(HSTADD)(a0)
	li	a1,1
	bal	rom_prx
	bal	rom_prnl
	
	li	a0,PA_TO_KVA1(ISAPORT_BASE(SMB_PORT))
	lbu	a0,SMBOFFS(HSTDAT0)(a0)
	li	a1,1
	bal	rom_prx
	bal	rom_prnl
	
	li	a0,PA_TO_KVA1(ISAPORT_BASE(SMB_PORT))
	lbu	a0,SMBOFFS(HSTDAT1)(a0)
	li	a1,1
	bal	rom_prx
	bal	rom_prnl
	
	li	a0,PA_TO_KVA1(ISAPORT_BASE(SMB_PORT))
	lbu	a0,SMBOFFS(BLKDAT)(a0)
	li	a1,1
	bal	rom_prx
	bal	rom_prnl
	
	li	a0,PA_TO_KVA1(ISAPORT_BASE(SMB_PORT))
	lbu	a0,SMBOFFS(SLVCNT)(a0)
	li	a1,1
	bal	rom_prx
	bal	rom_prnl
	
	li	a0,PA_TO_KVA1(ISAPORT_BASE(SMB_PORT))
	lbu	a0,SMBOFFS(SHDWCMD)(a0)
	li	a1,1
	bal	rom_prx
	bal	rom_prnl
	
	li	a0,PA_TO_KVA1(ISAPORT_BASE(SMB_PORT))
	lbu	a0,SMBOFFS(SLVEVT)(a0)
	li	a1,1
	bal	rom_prx
	bal	rom_prnl
	
	li	a0,PA_TO_KVA1(ISAPORT_BASE(SMB_PORT))
	lbu	a0,SMBOFFS(SLVDAT)(a0)
	li	a1,1
	bal	rom_prx
	bal	rom_prnl
	
	jr	t4
SEND(i2cdump)

#endif	



#include "reginit.s"		

	.rdata
	.align 3
#define BONITO_INIT(r,v)	WR_INIT(W,BONITO_BASE+BONITO_/**/r,v)
#define BONITO_BIS(r,b)		RMW_INIT(W,BONITO_BASE+BONITO_/**/r,~0,b)
#define BONITO_BIC(r,b)		RMW_INIT(W,BONITO_BASE+BONITO_/**/r,~(b),0)
#define BONITO_BISD(r,b)	RMW_INIT(W,BONITO_BASE+BONITO_/**/r,~0,b)
#define BONITO_BICD(r,b)	RMW_INIT(W,BONITO_BASE+BONITO_/**/r,~(b),0)
#define BONITO_RMW(r,c,s)	RMW_INIT(W,BONITO_BASE+BONITO_/**/r,~(c),s)
	

/*#define CFGADDR(idsel,function,reg) ((1<<(11+(idsel)))+((function)<<8)+(reg))*/
/* generic ISA Bridge (PIIX) configuration accesses */	
#define _ISABWR_INIT(mod,function,isabreg,val) \
	WR_INIT(W,BONITO_BASE+BONITO_PCIMAP_CFG,CFGADDR(PCI_DEV_I82371,function,isabreg)>>16) ; \
	RD_INIT(W,BONITO_BASE+BONITO_PCIMAP_CFG) ; \
	WR_INIT(mod,PCI_CFG_SPACE+(CFGADDR(PCI_DEV_I82371,function,isabreg)&0xffff),val)
#define _ISABRD_INIT(mod,function,isabreg) \
	WR_INIT(W,BONITO_BASE+BONITO_PCIMAP_CFG,CFGADDR(PCI_DEV_I82371,function,isabreg)>>16) ; \
	RD_INIT(W,BONITO_BASE+BONITO_PCIMAP_CFG) ; \
	RD_INIT(mod,PCI_CFG_SPACE+(CFGADDR(PCI_DEV_I82371,function,isabreg)&0xffff))

/* generic ISA byte I/O accesses */	
#define _ISAWR_INIT(isareg,val) \
	WR_INIT(B,PCI_IO_SPACE+(isareg),val)
#define _ISARD_INIT(isareg) \
	RD_INIT(B,PCI_IO_SPACE+(isareg))
	
/* ISA Bridge (PIIX) configuration accesses */
#if #endian(big)
#if defined(P6032)
/*
 * byte swapper disabled for config cycles
 * twiddle addresses but not data
 */	
#define ISABBWR_INIT(function,isabreg,val) \
	_ISABWR_INIT(B,function,(isabreg)^3,val)
#define ISABHWR_INIT(function,isabreg,val) \
	_ISABWR_INIT(H,function,(isabreg)^2,val)
#define ISABWWR_INIT(function,isabreg,val) \
	_ISABWR_INIT(W,function,(isabreg),val)
#else	
/*
 * byte swapper enabled for config cycles
 * twiddle data
 */	
#define HSWAP(h) ((((h) >> 8) & 0xff) | (((h) & 0xff) << 8))
#define WSWAP(w) (HSWAP(((w) >> 16) & 0xffff) | (HSWAP((w) & 0xffff) << 16))
#define ISABBWR_INIT(function,isabreg,val) \
	_ISABWR_INIT(B,function,(isabreg),val)
#define ISABHWR_INIT(function,isabreg,val) \
	_ISABWR_INIT(H,function,(isabreg),HSWAP(val))
#define ISABWWR_INIT(function,isabreg,val) \
	_ISABWR_INIT(W,function,(isabreg),WSWAP(val))
#endif		
#else
#define ISABBWR_INIT(function,isabreg,val) \
	_ISABWR_INIT(B,function,(isabreg),val)
#define ISABHWR_INIT(function,isabreg,val) \
	_ISABWR_INIT(H,function,(isabreg),val)
#define ISABWWR_INIT(function,isabreg,val) \
	_ISABWR_INIT(W,function,isabreg,val)
#endif
	
/* ISA I/O accesses */	
#if #endian(big) && defined(P6032)
/*
 * byte swapper disabled for I/O cycles
 * twiddle addresses
 */	 	
#define ISAWR_INIT(isareg,val) \
	_ISAWR_INIT((isareg)^3,val)
#define ISARD_INIT(isareg) \
	_ISARD_INIT((isareg)^3)
#else	
#define ISAWR_INIT(isareg,val) \
	_ISAWR_INIT(isareg,val)
#define ISARD_INIT(isareg) \
	_ISARD_INIT(isareg)
#endif
				
#define DISPLAY_INIT(d0,d1,d2,d3,d4,d5,d6,d7) \
	WR_INIT(W, LED_BASE+HD2532_CRAM+HD2532_CHAR(0), 0+d0); \
	WR_INIT(W, LED_BASE+HD2532_CRAM+HD2532_CHAR(1), 0+d1); \
	WR_INIT(W, LED_BASE+HD2532_CRAM+HD2532_CHAR(2), 0+d2); \
	WR_INIT(W, LED_BASE+HD2532_CRAM+HD2532_CHAR(3), 0+d3); \
	WR_INIT(W, LED_BASE+HD2532_CRAM+HD2532_CHAR(4), 0+d4); \
	WR_INIT(W, LED_BASE+HD2532_CRAM+HD2532_CHAR(5), 0+d5); \
	WR_INIT(W, LED_BASE+HD2532_CRAM+HD2532_CHAR(6), 0+d6); \
	WR_INIT(W, LED_BASE+HD2532_CRAM+HD2532_CHAR(7), 0+d7)

#define ISAREFRESH (PT_CRYSTAL/(1000000/15))

reginittab:
	/* use doubleword accesses to set bonito endianess */
#if #endian(big)
	BONITO_BISD(BONPONCFG,BONITO_BONPONCFG_CPUBIGEND)
	BONITO_BIS(BONGENCFG,BONITO_BONGENCFG_BYTESWAP|BONITO_BONGENCFG_MSTRBYTESWAP)
#if defined(P6032)	
	/* Bonito ASIC can't do subword accesses to I/O space with byteswapper enabled */
	BONITO_BIC(BONGENCFG,BONITO_BONGENCFG_MSTRBYTESWAP)
#endif	
#else
	BONITO_BICD(BONPONCFG,BONITO_BONPONCFG_CPUBIGEND)
	BONITO_BIC(BONGENCFG,BONITO_BONGENCFG_BYTESWAP|BONITO_BONGENCFG_MSTRBYTESWAP)
#endif
	BONITO_BIS(BONPONCFG, BONITO_BONPONCFG_IS_ARBITER|BONITO_BONPONCFG_PCIRESET_OUT)
	
	/* Bonito PIO initialisation */

        BONITO_INIT(GPIODATA,PIO_PIIXRESET)	# initial value
	BONITO_INIT(GPIOIE,PIO_IE)
	
	/* Clear PCI reset and enable PIIX */
	DELAY_INIT(ROMMS(1))
	BONITO_BIC(BONPONCFG,BONITO_BONPONCFG_PCIRESET_OUT)
	DELAY_INIT(ROMMS(2))
	BONITO_BIC(GPIODATA,PIO_PIIXRESET)
	DELAY_INIT(ROMMS(50))
	
	/* PCI bus and PIIX should now be usable */
	
#ifdef P6032
        /*
         * In certain situations it is possible for the Bonito ASIC
         * to come up with the PCI registers uninitialised, so do them here
         */
        BONITO_INIT(PCICLASS,(PCI_CLASS_BRIDGE << PCI_CLASS_SHIFT) | (PCI_SUBCLASS_BRIDGE_HOST << PCI_SUBCLASS_SHIFT))
	BONITO_INIT(PCICMD, BONITO_PCICMD_PERR_CLR|BONITO_PCICMD_SERR_CLR|BONITO_PCICMD_MABORT_CLR|BONITO_PCICMD_MTABORT_CLR|BONITO_PCICMD_TABORT_CLR|BONITO_PCICMD_MPERR_CLR)
	BONITO_INIT(PCILTIMER, 0)
	BONITO_INIT(PCIBASE0, 0)
	BONITO_INIT(PCIBASE1, 0)
	BONITO_INIT(PCIBASE2, 0)
	BONITO_INIT(PCIEXPRBASE, 0)
	BONITO_INIT(PCIINT, 0)
#endif
	
	BONITO_BIS(PCICMD, BONITO_PCICMD_PERRRESPEN)
	
	BONITO_BIS(PCICMD, PCI_COMMAND_IO_ENABLE|PCI_COMMAND_MEM_ENABLE|PCI_COMMAND_MASTER_ENABLE)
	
	/* enable i/o buffer cache and other go faster bits */
	BONITO_BIS(BONGENCFG, \
			BONITO_BONGENCFG_BUSERREN| \
			BONITO_BONGENCFG_PREFETCHEN| \
			BONITO_BONGENCFG_WBEHINDEN| \
			BONITO_BONGENCFG_PCIQUEUE| \
			BONITO_BONGENCFG_SNOOPEN)

	/* Set debug mode */
	BONITO_BIS(BONGENCFG, BONITO_BONGENCFG_DEBUGMODE)

	/* FIXME - probably more needed */
#ifdef P6032
	BONITO_BIS(IODEVCFG, BONITO_IODEVCFG_MOREABITS_CS1|\
			     BONITO_IODEVCFG_SPEEDBIT_CS2|BONITO_IODEVCFG_BUFFBIT_CS2|\
			     BONITO_IODEVCFG_SPEEDBIT_CS3|BONITO_IODEVCFG_BUFFBIT_CS3|\
			     BONITO_IODEVCFG_BUFFBIT_IDE)
#endif
#ifdef P6064
	BONITO_BIS(IODEVCFG, BONITO_IODEVCFG_BUFFBIT_CS0|\
			     BONITO_IODEVCFG_BUFFBIT_CS1|\
			     BONITO_IODEVCFG_SPEEDBIT_CS2|BONITO_IODEVCFG_BUFFBIT_CS2|\
			     BONITO_IODEVCFG_SPEEDBIT_CS3|BONITO_IODEVCFG_BUFFBIT_CS3|\
			     BONITO_IODEVCFG_BUFFBIT_IDE)
#endif
		
	/* switch on the LED */
	WR_INIT(W, LED_BASE+HD2532_CW,HD2532_CW_C)
	DELAY_INIT(ROMUS(110))
	
	/* 
	 * Initialise the ISA bridge via its CONF space
	 */
	
	DISPLAY_INIT ('I','s','a','B','r','i','d','g')
	
	/* Turn most special purpose pins into GPIO; set ISA mode */
	ISABWWR_INIT(0, I82371_GENCFG, I82371_GENCFG_CFG)
	
	/* disable RTC & KBD chip selects */
	ISABHWR_INIT(0, I82371_XBCS, 0)
	
	/* Enable PCI 2.1 timing support */
	ISABBWR_INIT(0, I82371_DLC, I82371_DLC_DT /* | I82371_DLC_PR */ | I82371_DLC_USBPR | I82371_DLC_DTTE)

	/* Set top of memory to 16MB, so all ISA bus master & DMA
	   accesses are forwarded to PCI mem space
	 */
	ISABBWR_INIT(0, I82371_TOM, I82371_TOM_TOM(16) | I82371_TOM_FWD_LBIOS | I82371_TOM_FWD_AB | I82371_TOM_FWD_89)
	
	/* disable the internal RTC */
	ISABBWR_INIT(0, I82371_RTCCFG, 0);	
	
	/* Set the SMB base address */
	ISABWWR_INIT(3, I82371_PCI3_SMBBA, SMB_PORT|PCI_MAP_IO)
	/* enable the host controller */
	ISABBWR_INIT(3, I82371_PCI3_SMBHSTCFG, I82371_PCI3_SMB_HST_EN)
	/* enable the SMB IO ports */
	ISABBWR_INIT(3, PCI_COMMAND_STATUS_REG, PCI_COMMAND_IO_ENABLE)

	/* Set the PIIX power management base address */
	ISABWWR_INIT(3, I82371_PCI3_PMBA, PM_PORT|PCI_MAP_IO)
	/* enable the power management ports */
	ISABBWR_INIT(3, I82371_PCI3_PMREGMISC, I82371_PCI3_PMIOSE)

	/* Initialise ISA bus low-level I/O devices */
	DISPLAY_INIT('I','s','a','D','e','v',' ',' ')

/* 15us ISA bus refresh clock */
#define ISAREFRESH (PT_CRYSTAL/(1000000/15))
	
	ISARD_INIT(CTC_PORT+PT_CONTROL)
	
	/* program i8254 ISA refresh counter */
	ISAWR_INIT(CTC_PORT+PT_CONTROL,PTCW_SC(PT_REFRESH)|PTCW_16B|PTCW_MODE(MODE_RG))
	ISAWR_INIT(CTC_PORT+PT_REFRESH, ISAREFRESH & 0xff)
	ISAWR_INIT(CTC_PORT+PT_REFRESH, ISAREFRESH >> 8)

	/* program ISA ICU */
	ISAWR_INIT(ICU1_PORT,  0x11) /* ICW1 */
	ISAWR_INIT(ICU1_PORT+1,0x00) /* ICW2: vector */
	ISAWR_INIT(ICU1_PORT+1,0x04) /* ICW3: cascade on IRQ2 */
	ISAWR_INIT(ICU1_PORT+1,0x01) /* ICW4: 8086 mode */
	ISAWR_INIT(ICU1_PORT+1,0xff) /* OCW1: mask all */

	ISAWR_INIT(ICU2_PORT,  0x11) /* ICW1 */
	ISAWR_INIT(ICU2_PORT+1,0x08) /* ICW2: vector */
	ISAWR_INIT(ICU2_PORT+1,0x02) /* ICW3:  */
	ISAWR_INIT(ICU2_PORT+1,0x01) /* ICW4: 8086 mode */
	ISAWR_INIT(ICU2_PORT+1,0xff) /* OCW1: mask all */
		
	ISAWR_INIT(ICU1_PORT+1,~(1<<2)) /* enable IRQ2 */

	/* set up ISA devices */

	/* select logical device 1 (mouse) */	
	ISAWR_INIT(ISAPNP_MBADDR,ISAPNP_LOGICAL_DEV_NUM)
	ISAWR_INIT(ISAPNP_MBDATA,1)
	ISAWR_INIT(ISAPNP_MBADDR,ISAPNP_ACTIVATE)
	ISAWR_INIT(ISAPNP_MBDATA,1)
	
	/* select logical device 4 (parallel) */	
	ISAWR_INIT(ISAPNP_MBADDR,ISAPNP_LOGICAL_DEV_NUM)
	ISAWR_INIT(ISAPNP_MBDATA,4)
	ISAWR_INIT(ISAPNP_MBADDR,ISAPNP_IO_DESC0+ISAPNP_IO_BASE_15_8)
	ISAWR_INIT(ISAPNP_MBDATA,(ECP_PORT>>8) & 0xff)
	ISAWR_INIT(ISAPNP_MBADDR,ISAPNP_IO_DESC0+ISAPNP_IO_BASE_7_0)
	ISAWR_INIT(ISAPNP_MBDATA,ECP_PORT & 0xff)
	ISAWR_INIT(ISAPNP_MBADDR,ISAPNP_IRQ_DESC0+ISAPNP_IRQ_CONTROL)
	ISAWR_INIT(ISAPNP_MBDATA,ISAPNP_IRQ_HIGH)
	ISAWR_INIT(ISAPNP_MBADDR,ISAPNP_ACTIVATE)
	ISAWR_INIT(ISAPNP_MBDATA,1)
	
	/* select logical device 5 (COM2) */	
	ISAWR_INIT(ISAPNP_MBADDR,ISAPNP_LOGICAL_DEV_NUM)
	ISAWR_INIT(ISAPNP_MBDATA,5)
	ISAWR_INIT(ISAPNP_MBADDR,ISAPNP_ACTIVATE)
	ISAWR_INIT(ISAPNP_MBDATA,1)
	
	/* select logical device 6 (COM1) */	
	ISAWR_INIT(ISAPNP_MBADDR,ISAPNP_LOGICAL_DEV_NUM)
	ISAWR_INIT(ISAPNP_MBDATA,6)
	ISAWR_INIT(ISAPNP_MBADDR,ISAPNP_ACTIVATE)
	ISAWR_INIT(ISAPNP_MBDATA,1)
	
#if 0
	/* select logical device 7 (PIO) */	
	ISAWR_INIT(ISAPNP_MBADDR,ISAPNP_LOGICAL_DEV_NUM)
	ISAWR_INIT(ISAPNP_MBDATA,7)
	ISAWR_INIT(ISAPNP_MBADDR,ISAPNP_IO_DESC0+ISAPNP_IO_BASE_15_8)
	ISAWR_INIT(ISAPNP_MBDATA,(GPIO_PORT>>8) & 0xff)
	ISAWR_INIT(ISAPNP_MBADDR,ISAPNP_IO_DESC0+ISAPNP_IO_BASE_7_0)
	ISAWR_INIT(ISAPNP_MBDATA,GPIO_PORT & 0xff)
	ISAWR_INIT(ISAPNP_MBADDR,ISAPNP_ACTIVATE)
	ISAWR_INIT(ISAPNP_MBDATA,1)
	
	/* set default configuration for GPIO2 */
	ISAWR_INIT(GPIO_PORT+GPIO2_OTYPE, 0xff)
	ISAWR_INIT(GPIO_PORT+GPIO2_PULLUP, 0)
	ISAWR_INIT(GPIO_PORT+GPIO2_DIR, PIO_OMASK)
	ISAWR_INIT(GPIO_PORT+GPIO2_DATA, 0)
#endif
		
	/* Completed */
	DISPLAY_INIT('I','n','i','t','d','o','n','e')
		
	EXIT_INIT(0)
	
	.text

	
#ifdef IN_PMON
	
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
	la	t0,wbfltmp
	or	t0,K1BASE
	sync
	lw	zero,0(t0)
	j	ra
END(wbflush)

	
LEAF(sbddelay)
	mfc0	t0,C0_CONFIG
	li	t1,CACHEMISS+ROMCYCLE
	and	t0,CFG_K0MASK
	beq	t0,CFG_C_UNCACHED,1f
	and	t0,ra,0x20000000
	bnez	t0,1f
	li	t1,CACHECYCLE
1:	mul	a0,1000
	addu	a0,t1
	sll	t1,1
	divu	a0,t1
	subu	a0,48		# approx number of loops so far
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
	
#endif /* IN_PMON */
