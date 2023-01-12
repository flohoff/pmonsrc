/*
 * devaz/sbdreset.S: low level reset code for DEVA-0
 * Copyright (c) 1999 Algorithmics Ltd.
 */

#define DEVAZ_I2C
#undef	DEVAZ_PARITY
		
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
#include <mips/prid.h>
	
#if !__mips64	
	/* force 64-bit register support */
	.set	gp64
#endif
	
/* CPU-specific bootmode bitstreams */	
#if #endian(big)	
#define BOOTMODE_RM52XX		0x00230100
#define BOOTMODE_R4650		0x00001100
#define BOOTMODE_RC6447X	0x00001100
#define BOOTMODE_RC6457X	0x06001100
#else
#define BOOTMODE_RM52XX		0x00230000
#define BOOTMODE_R4650		0x00001000
#define BOOTMODE_RC6447X	0x00001000
#define BOOTMODE_RC6457X 	0x06001000
#endif
	
#define WBFLUSH 

#define MEG	0x100000		
	
/* No display */	
#define DISPLAY(c1,c2,c3,c4) 
	
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
#define gpiodata	s4
#define sdCfg		s5
#define	rasave		s7

LEAF(_sbd_reset)
	move	rasave,ra
	
	mfc0	t1,C0_CONFIG
	mfc0	t2,C0_PRID		# get PrID
	mtc0	zero,$18		# C0_IWATCH/C0_WATCHLO
	mtc0	zero,$19		# C0_DWATCH/C0_WATCHHI
	
	/* get global pointer to Bonito registers */
	li	bonito,PA_TO_KVA1(BONITO_BASE)
	
	srl	t2,8

	/* check for CPUs which need a non-zero bootmode stream */
#ifdef BOOTMODE_RM52XX
1:	bne	t2,PRID_RM52XX,1f
	li	t3,BOOTMODE_RM52XX
	b	.bootcheck
#endif	
#ifdef BOOTMODE_RC6447X
1:	bne	t2,PRID_RC6447X,1f
	li	t3,BOOTMODE_RC6447X
	b	.bootcheck
#endif
#ifdef BOOTMODE_RC6457X
1:	bne	t2,PRID_RC6457X,1f
	li	t3,BOOTMODE_RC6457X
	b	.bootcheck
#endif
#ifdef BOOTMODE_R4650
1:	bne	t2,PRID_R4650,1f
	li	t3,BOOTMODE_R4650
	b	.bootcheck
#endif
1:	b	.noreset
	
.bootcheck:	
	/* does bootmode match value in bonito intedge cfg register? */
	lw	t0,BONITO_INTPOL(bonito)
	beq	t0,t3,.noreset

	/* no, we must reset and reboot */
	sw	t3,BONITO_INTPOL(bonito)
	lw	t3,BONITO_BONGENCFG(bonito)
	or	t3,BONITO_BONGENCFG_CPUSELFRESET
	sw	t3,BONITO_BONGENCFG(bonito)
	lw	t3,BONITO_BONGENCFG(bonito)
1:	b	1b		# loop forever

.noreset:
	/* if we get here, the cpu has been correctly initialised */
	bne	t2,PRID_R4650,2f
	
	/* r4640/50 initialisation */
	mtc0	zero,$0			# C0_IBASE
	mtc0	zero,$1			# C0_IBOUND
	mtc0	zero,$2			# C0_DBASE
	mtc0	zero,$3			# C0_DBOUND
#ifndef ITBASE	
	li	t3,0x22233333
	mtc0	t3,$17			# C0_CALG
#endif	
	b	3f
	
2:	/* r4xxx/r5xxx initialisation */
#ifndef ITBASE	
	/* set config register for 32b/32b cachelines, kseg0 cacheable */
	and	t1,~0x3f		# set bits 5..0 only
	or	t1,CFG_IB | CFG_DB | CFG_C_WBACK
	/* Vr4300/5432: software controls the endianness (r/o on other CPUs) */
#if #endian(big)
	or	t1,CFG_BE	
#else
	and	t1,~CFG_BE	
#endif
	/* set DDDD rate for CPUs that aren't hardware configured */
	and	t1,~CFG_EPMASK
	or	t1,CFG_EPD
	bne	t2,PRID_R5400,1f
	/* Vr5400: set R4x00 compatible bus mode */
	and	t1,~CFG_EMMASK
	or	t1,CFG_EM_R4K
	b	2f
	/* Vr4100: switch off accelerated data (undefined on other CPUs)*/
1:	and	t1,~CFG_AD
2:	mtc0	t1,C0_CONFIG
#endif
	
3:	mfc0	t1,C0_STATUS		# get Status
	mtc0	zero,C0_CAUSE
	and	t1,SR_SR		# leave the SoftReset bit
	or	t1,SR_BEV		# force Boot Exception Vec
	mtc0	t1,C0_STATUS
	
	/* Initialise other low-level I/O devices */
	la	a0,reginittab
	addu	a0,RELOC
	bal	reginit

#ifdef DEVAZ_I2C
	/* 
	 * Now determine DRAM configuration and size by
	 * reading the I2C EEROM on the SODIMM
	 */

	bal	i2creset
	li	msize,0
	
	DISPLAY ('D','I','M','0')
	
	/* start with SODIMM #0 */
	li	a0,0
	li	sdCfg,0
	
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
	
2:	addu	msize,tmpsize
	b	2f
		
.nodimm:
	or	sdShape,BONITO_SDCFG_AABSENT
2:	beqz	a0,1f
	sll	sdShape,BONITO_SDCFG_BROWBITS_SHIFT
1:	or	sdCfg,sdShape
	
#ifdef notdef /* only one DIMM on this board */
	DISPLAY ('D','I','M','1')
	addu	a0,1
	bltu	a0,2,.nextdimm
#endif		
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
	
2:		
#else	
	li	msize,0
#endif
	
	/* XXX invalidate i/o buffer cache */
	
	li	t1,0		# accumulate pcimembasecfg settings
		
	/* set bar0 mask and translation to point to SDRAM */
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

	
	/* set bar1 to minimum size to allow P4032 to configure us */
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
			
#ifdef DEVAZ_PARITY	
	/* We have to clear memory to initialise parity */

	/* Skip memory clear if no memory */
	beqz	msize,.noclear
	
	/* Skip memory clear if a soft reset */
	mfc0	t1,C0_STATUS
	and	t1,SR_SR
	bnez	t1,.noclear
	
	/* Clear bottom 64K running uncached */
	DISPLAY ('Z','6','4','K')
	li	a0,PA_TO_KVA1(0)
	addu	a1,a0,65536
	.set noreorder
1:	sd	zero,0(a0)
	sd	zero,8(a0)
	sd	zero,16(a0)
	addu	a0,32
	bne	a0,a1,1b
	sd	zero,-8(a0)
	.set reorder
	
	/* We can now initialise the caches for a fast clear_mem */
	DISPLAY ('C','A','C','H')
#ifdef IN_PMON	
	jal	mips_init_cache	
#else	
	bal	mips_init_cache	
#endif
		
	/* Finally clear rest of memory running cached */
	li	a0,PA_TO_KVA1(65536)
	subu	a1,msize,65536
	blez	a1,.noclear
	addu	a1,a0
	
	/* clear all of memory (to set correct parity) */
	DISPLAY ('Z','M','E','M')
	
	/* run memory clear loop cached */
	la	t0,1f
	addu	t0,RELOC
	and	t0,~0x20000000
	j	t0
	
	.set noreorder
1:	#cache	Create_Dirty_Exc_D,0(a0)
	sd	zero,0(a0)
	sd	zero,8(a0)
	sd	zero,16(a0)
	addu	a0,32
	bne	a0,a1,1b
	sd	zero,-8(a0)
	.set reorder

	/* revert to uncached */
	la	t0,.noclear
	addu	t0,RELOC
	j	t0
.noclear:
		
#endif /* DEVAZ_PARITY */
		
	/* return to generic code, with available memory size */
	DISPLAY ('R','U','N',' ')
	move	ra,rasave
#ifdef IN_PMON		
	sw	msize,PA_TO_KVA1(0x180-4)
#else	
	move	v0,msize
#endif
	j	ra
END(_sbd_reset)


LEAF(_sbd_memfail)
	DISPLAY ('!','M','E','M')
1:	b	1b
	j	ra
END(_sbd_memfail)


		
/*
 * This is the code to read the I2C EEROM on the DIMMS
 */
	
#define EEABITS		8	/* 256x8 = 8 address bits */
#define PGSHIFT		7
#define PGMASK		(((0x1 << (EEABITS-8)) - 1) << 1)
#define PGNUM(o)	(((o) >> PGSHIFT) & PGMASK)
#define OMASK		0xff
#define OFFS(o)		((o) & OMASK)
#define READ		0x01
#define WRITE		0x00

#define	SET \
	sw	gpiodata,BONITO_GPIODATA(bonito);
	
#define	BIS(x) \
	or	gpiodata,(x); SET

#define	BIC(x) \
	and	gpiodata,~(x); SET
	
/*
 * Max chip frequency = 100 kHz, so we require at least 5 usec
 * delay between signal changes.  Assume running in RAM for
 * worst case timings.
 */
#define WAIT \
	li	t9,RAMUS(6); \
	.set noreorder; \
10:	bnez	t9,10b; \
	subu	t9,1; \
	.set reorder
	
/* set SCL high */
#define SCL_HI \
	WAIT; BIS(PIO_I2C_SCL)
	
/* set SCL low */
#define SCL_LO \
	WAIT; BIC(PIO_I2C_SCL)
	
/* set SDA high */
#define SDA_HI \
	WAIT; BIS(PIO_I2C_DIMMSDAW)
	
/* set SDA low */
#define SDA_LO \
	WAIT; BIC(PIO_I2C_DIMMSDAW)
	
/* disable SDA output driver */
#define SDI \
	WAIT; \
	and	gpiodata,~PIO_I2C_DIMMSDAW; \
	li	t9,PIO_IE; \
	sw	t9,BONITO_GPIOIE(bonito); \
	sw	gpiodata,BONITO_GPIODATA(bonito)
	
/* enable SDA output driver */
#define SDO \
	WAIT; \
	or	gpiodata,PIO_I2C_DIMMSDAW; \
	li	t9,PIO_IE & ~PIO_I2C_DIMMSDAW; \
	sw	t9,BONITO_GPIOIE(bonito); \
	sw	gpiodata,BONITO_GPIODATA(bonito)

/* read SDA */
#define SDRD \
	WAIT; \
	lw	v0,BONITO_GPIODATA(bonito); \
	and	v0,PIO_I2C_DIMMSDAR; \
	sltu	v0,zero,v0
	
/* send a START: SDA high->low when SCL high */
#define START \
	SDO;	\
	SDA_HI; \
	SCL_HI; \
	SDA_LO; \
	SCL_LO

/* send a STOP: SDA low->high when SCL high */
#define STOP \
	SDO;	\
	SDA_LO; \
	SCL_HI; \
	SDA_HI; \
	SCL_LO; \
	SDA_LO; \
	SDI

/* receive an ACK: a single 0 bit */
#define GETACK \
	SCL_HI; \
	SDRD;   \
	SCL_LO; \
	xor v0,1

/* send an ACK: a single 0 bit */
#define SENDACK \
	SDO;	\
	SDA_LO; \
	SCL_HI; \
	SCL_LO; \
	SDI
	
/* send a NACK: a single 1 bit */
#define SENDNACK \
	SDO;	\
	SDA_HI; \
	SCL_HI; \
	SCL_LO; \
	SDA_LO; \
	SDI

/* send 8 bit word in ta0 (note: check for ACK externally) */
SLEAF(i2csend8)
	li	t0,8
	SDO
1:	and	t1,ta0,0x80
	beqz	t1,2f
	SDA_HI
	b	3f
2:	SDA_LO
3:	SCL_HI
	SCL_LO
	sll	ta0,1
	subu	t0,1
	bnez	t0,1b
	SDA_LO
	SDI
	j	ra
SEND(i2csend8)


/* receive 8 bit word into v0 (note: send ACK/NACK externally) */
SLEAF(i2cget8)
	li	t1,0
	li	t0,7
1:	SCL_HI
	SDRD
	SCL_LO
	sll	v0,t0
	or	t1,v0
	subu	t0,1
	bgez	t0,1b
	move	v0,t1
	j	ra
SEND(i2cget8)
	

/* DADDR(rw, a0=dev, a1=addr)
	send first command byte: device address & page number */
	
#define DADDR(rw)						\
	START;							\
	sll t0,a0,1; 		/* t0 = dev << 1 */		\
	srl ta0,a1,PGSHIFT;	/* ta0 = (addr >> PGSHIFT) */	\
	and ta0,PGMASK;		/* ta0 &= PGMASK */		\
	or ta0,t0; 		/* ta0 |= (dev << 1) */		\
	or ta0,(0xa0 | rw);	/* ta0 |= (0xa0 | rw) */	\
	bal i2csend8;						\
	GETACK

/* ADDR(rw, a0=dev, a1=addr)
	send two command bytes */
	
#define ADDR(rw)						\
	/* timeout for previous write to complete */		\
	li t3,1000; 						\
								\
	/* send first byte: device address */ 			\
1:	DADDR(rw); 						\
	bnez v0, 2f; 						\
	/* no acknowledge - count down */ 			\
	subu t3,1; 						\
	beqz t3,99f; 						\
	b 1b; 							\
								\
2:	/* send second byte: word offset */ 			\
	and ta0,a1,OMASK;	/* offset in page */ 		\
	bal i2csend8;						\
	GETACK;							\
99:

	
SLEAF(i2creset)
	/* initialise global registers */
	li	gpiodata,0
	
        /* send 9 STOPS */
	li	t0,9
1:	STOP
	subu	t0,1
	bnez	t0,1b
	
	j	ra
SEND(i2creset)
	

/* i2cread (unsigned dev, unsigned offs) */
SLEAF(i2cread)	
	move	t8,ra
	
	ADDR(WRITE)		# write address
	beqz	v0,9f		# ack?
	
	DADDR(READ)		# start read
	beqz	v0,9f		# ack?
	
	bal	i2cget8		# get data byte
	
	SENDNACK		# terminate read
	STOP
	
	j	t8
	
9:	li	v0,-1
	j	t8
SEND(i2cread)	
	


#include "reginit.s"		

	.rdata
#define BONITO_INIT(r,v) WR_INIT(W,BONITO_BASE+BONITO_/**/r,v)
#define BONITO_BIS(r,b) RMW_INIT(W,BONITO_BASE+BONITO_/**/r,~0,b)
#define BONITO_BIC(r,b) RMW_INIT(W,BONITO_BASE+BONITO_/**/r,~(b),0)
#define BONITO_RMW(r,c,s) RMW_INIT(W,BONITO_BASE+BONITO_/**/r,~(c),s)
	
reginittab:
	/* bonito endianess */
#if #endian(big)
	BONITO_BIS(BONPONCFG,BONITO_BONPONCFG_CPUBIGEND)
	BONITO_BIS(BONGENCFG,BONITO_BONGENCFG_BYTESWAP|BONITO_BONGENCFG_MSTRBYTESWAP)
#else
	BONITO_BIC(BONPONCFG,BONITO_BONPONCFG_CPUBIGEND)
	BONITO_BIC(BONGENCFG,BONITO_BONGENCFG_BYTESWAP|BONITO_BONGENCFG_MSTRBYTESWAP)
#endif
	
	/* clear self reset & enable i/o buffer cache */
	BONITO_RMW(BONGENCFG,BONITO_BONGENCFG_CPUSELFRESET, \
		BONITO_BONGENCFG_PREFETCHEN|BONITO_BONGENCFG_WBEHINDEN \
		|BONITO_BONGENCFG_PCIQUEUE|BONITO_BONGENCFG_SNOOPEN)
	
	/* PIO input enables */
	BONITO_INIT(GPIOIE,PIO_IE)
	
	/* Reset UART */
	BONITO_INIT(GPIODATA,0)
	DELAY_INIT(RAMUS(1))
	BONITO_INIT(GPIODATA,PIO_UART_RESET)
	DELAY_INIT(RAMUS(1))
	BONITO_INIT(GPIODATA,0)
	DELAY_INIT(RAMUS(1))

	/* enable IDE data buffers */
	BONITO_BIS(IODEVCFG, BONITO_IODEVCFG_BUFFBIT_CS3|BONITO_IODEVCFG_BUFFBIT_CS2)
	/* and enable IDE */
	BONITO_BIS(GPIODATA, PIO_IDE_RESET);
	
	EXIT_INIT(0)
	

	
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
