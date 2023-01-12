/* $Id: sbdreset.S,v 1.3 1997/10/09 23:01:19 chris Exp $ */
/*
 * ALGVME.S: common low level code for Algorithmics' VME R3000/R4000 boards
 * Copyright (c) 1992 Algorithmics Ltd.
 */

#include <mips.h>
#include <pmon.h>
#include <algvme/sbd.h>
#include <algvme/vic068.h>
#include <algvme/vac068.h>

#if #cpu(r4000)
#define WBFLUSH sync
#else
#define WBFLUSH \
	.set noreorder;		\
	nop;nop;nop;nop; 	\
9:	bc0f 9b; 		\
	nop;			\
	.set reorder
#endif

#ifndef M_BUSWIDTH
# if #cpu(r4000)
#  define M_BUSWIDTH	16
# else
#  define M_BUSWIDTH	8
# endif
#endif

#define MEMINCR		0x100000

#define DELAY(n) \
        .set    noreorder;	\
        .set    noat; 		\
        li      AT,(n); 	\
9:	bnez    AT,9b;	 	\
        subu    AT,1; 		\
        .set    at; 		\
        .set    reorder

/*
 * rough scaling factors for ms delays
 */
#if #cpu(r4000)
#define CACHEMS 1000000/(3*10)          /* CACHE - 10ns instruction XXX */
#define RAMMS   1000000/(3*240)         /* RAM - 240ns instruction XXX */
#define ROMMS   1000000/(3*1080)        /* ROM - 1.08us instruction XXX */
#else
#define CACHEMS 1000000/(2*30)          /* CACHE - 30ns instruction XXX */
#define RAMMS   1000000/(2*240)         /* RAM - 240ns instruction XXX */
#define ROMMS   1000000/(2*1080)        /* ROM - 1.08us instruction XXX */
#endif

#define MSDELAY(x)	DELAY((x) * ROMMS)
	
/* Display 4 characters on alphanumeric display.
 * or 1 hex digit on SL3000 led display
 */
#ifdef SL3000
#define DISPLAY(d0, d1, d2, d3, n)	\
	li	t6,PHYS_TO_K1(LED_BASE); \
	li	t7,(n)<<24;		\
	sw	t7,0(t6);		\
	WBFLUSH
#else
#define DISPLAY(d0, d1, d2, d3, n)	\
	li	t6,PHYS_TO_K1(ALPHA_BASE); \
	li	t7,(d0)<<24; 		\
	sw	t7,ALPHA_ENTRY(0)(t6); 	\
	li	t7,(d1)<<24; 		\
	sw	t7,ALPHA_ENTRY(1)(t6); 	\
	li	t7,(d2)<<24; 		\
	sw	t7,ALPHA_ENTRY(2)(t6); 	\
	li	t7,(d3)<<24; 		\
	sw	t7,ALPHA_ENTRY(3)(t6); 	\
	WBFLUSH
#endif


	.globl	sbdreset
	.ent	sbdreset
sbdreset:
	move	s0,ra			# save return address

#ifdef R4000
	/* Disable "cache errors" caused by memory parity errors
	 * and ensure kseg 0 is set to "cached, non coherent" */
	.set	noreorder
	mfc0	s1,C0_SR
	mfc0	t1,C0_CONFIG
	nop
	or	t0,s1,SR_DE
	mtc0	t0,C0_SR
	and	t1,~CFG_K0MASK
	or	t1,CFG_C_NONCOHERENT
	mtc0	t1,C0_CONFIG
	.set	reorder 
#endif

	/* write to rom to force VAC out of eprom mode */
	sw	zero,PHYS_TO_K1(LOCAL_PROM)
	WBFLUSH

	/* program VAC with register init table */
	la	a0,vacresettab
	or	a0,K1BASE		/* force to kseg1 */
	
1:	lw	v0,0(a0)	# reg addr
	lw	v1,4(a0)	# reg data
	beqz	v0,2f
	sw	v1,0(v0)
	addu	a0,8
	b	1b
2:

	/* Initialise BCRR */	
	sw	zero,PHYS_TO_K1(BCRR)
	WBFLUSH
	MSDELAY(20)
#ifdef SL3000
	li	v0,BCRR_ETH|BCRR_SCSI
#else
	li	v0,BCRR_AUBLK|BCRR_AUCLR|BCRR_ETH|BCRR_SCSI
#endif
	sw	v0,PHYS_TO_K1(BCRR)
	WBFLUSH
	MSDELAY(20)

    /*
     * clear vic sysfail
     */
	move	s1,k0			/* save exception pointer */
	bal	1f
1:	subu	k0,ra,8			/* loop on any exception */

	lw	v0,VIC_VCONTROL
	and	v0,~VIC_SYSFMASK
	sw	v0,VIC_VCONTROL
	WBFLUSH

	lw	v0,VIC_VSTATUS
	and	v0,~VIC_SYSFAIL
	sw	v0,VIC_VSTATUS
	WBFLUSH

	move	k0,s1			/* restore exception handler */
	
	/*
	 * slight amount of kludgery here to stop RAM resident
	 * program from overwriting itself...
	 */
	li	v1,0x1fc00000		/* check return address is in ROM */
	and	v0,ra,v1
	bne	v0,v1,8f

	/*
	 * explicitly clear parity in bottom 64K for primary cache 
	 * initialisation (XXX what about secondary cache)
	 */
	DISPLAY ('Z','6','4','K', 0x1)
	li	t0,PHYS_TO_K1(0)
	li	t5,PHYS_TO_K1(0x10000)

	.set	noreorder
1:	sw	zero,0(t0)
	sw	zero,4(t0)
	sw	zero,8(t0)
	addu	t0,16
	bne	t0,t5,1b
	sw	zero,-4(t0)		# BDSLOT
	.set	reorder

	/*
	 * now we can safely initialise the caches
	 */
	DISPLAY ('C','A','C','H', 0x2)
#ifdef R4000
	jal	mips_init_cache
#else
	jal	r3k_iflush
	jal	r3k_dflush
#endif

#ifdef R4000
	/* skip memory size/clear if a soft reset */
	and	t0,s1,SR_SR
	bnez	t0,8f
#endif

	/*
	 * we *must* now size and clear all of memory, so that it
	 * has good parity
	 */
	DISPLAY ('S','M','E','M',0x0)
	li	t0, PHYS_TO_K1(0)	# start of test
	move	t1, t0			# bottom of memory
	li	t2, 0xaa55aa55		# test pattern
	not	t3, t2			# ~(test pattern)

memprobe:
        addu    t0, MEMINCR
	move	t4, t0

	/* store pattern in bank 0, line 0 */
	sw	t2, 0(t4)
	addu	t4, 4

#if M_BUSWIDTH > 4
	/* fill remainder of line with zeros */
	li	t5, M_BUSWIDTH-4
1:	sw	zero, 0(t4)
	subu	t5,4
	addu	t4,4
	bnez	t5,1b
#endif

	/* store inverse pattern in bank 0, line 1 */
	sw	t3, 0(t4)
	addu	t4, 4

#if M_BUSWIDTH > 4
	/* fill remainder of line with zeros */
	li	t5, M_BUSWIDTH-4
1:	sw	zero, 0(t4)
	subu	t5,4
	addu	t4,4
	bnez	t5,1b
#endif

	/* defeat write buffering */
	WBFLUSH

	lw	t4,0(t0)		# first word of line
	lw	t5,0(t1)		# start of memory
	bne	t4,t2,memfail		# first word wrong?
        beq     t5,zero,memprobe        # wrap-around clobbered start?

memfail:
	/* check for no (< mimimum) memory */
	bgtu	t0,PHYS_TO_K1(MEMINCR),1f
	DISPLAY ('0','M','E','M',0x0)
	b	sbdflash

1:	DISPLAY ('Z','M','E','M',0x0)
	li	t1,PHYS_TO_K1(0)
	/* force memory clear loop to run cached */
	la	t4,1f
	and	t4,~K1SIZE
	jr	t4

	/* XXX could use "sd" on R4000 for even faster clear */
	.set	noreorder
1:	sw	zero,0(t1)
	sw	zero,4(t1)
	sw	zero,8(t1)
	sw	zero,12(t1)
	sw	zero,16(t1)
	sw	zero,20(t1)
	sw	zero,24(t1)
	addu	t1,32
	bne	t1,t0,1b
	sw	zero,-4(t1)		# BDSLOT
	.set	reorder

8:
	DISPLAY ('D','O','N','E', 0x5)
#ifdef R4000
	mtc0	s1,C0_SR
#endif
	move	ra,s0
	j	ra
	.end	sbdreset

	.globl	sbddisplay
	.ent	sbddisplay
sbddisplay:
#ifdef SL3000
	sll	a1,24
	sw	a1,PHYS_TO_K1(LED_BASE)
#else
	la	t0,PHYS_TO_K1(ALPHA_DISP(0))
1:	sw	a0,0(t0)
	subu	t0,4		# display addressing goes backwards!
	sll	a0,8		# get next character into top byte
	bnez	a0,1b		# continue til all done
#endif

	/* unblank the display */
	la	t1,PHYS_TO_K1(BCRR)
	lw	t0,(t1)
#ifdef SL3000
	and	t0,~BCRR_LBLK
#else
	or	t0,BCRR_AUBLK
#endif
	sw	t0,(t1)
	WBFLUSH

	j	ra
	.end	sbddisplay



	.globl	sbdblank
	.ent	sbdblank
sbdblank:
	lw	t0,PHYS_TO_K1(BCRR)
	bnez	a0,blank
#ifdef SL3000
	and	t0,~BCRR_LBLK
	b	1f
blank:	or	t0,BCRR_LBLK
#else
	or	t0,BCRR_AUBLK
	b	1f
blank:	and	t0,~BCRR_AUBLK
#endif
1:	sw	t0,PHYS_TO_K1(BCRR)
	WBFLUSH
	j	ra
	.end	sbdblank


	.globl	sbdflash
	.ent	sbdflash
sbdflash:
	la	t0,PHYS_TO_K1(BCRR)
	lw	t1,0(t0)
	la	t2,1f
	or	t2,K1BASE
	jr	t2

#ifdef SL3000
1:	xor	t1,BCRR_LBLK
#else
1:	xor	t1,BCRR_AUBLK
#endif
	sw	t1,0(t0)
	MSDELAY(250)
	b	1b
	.end	sbdflash


	.globl	sbdparityenb
	.ent	sbdparityenb
sbdparityenb:
#ifdef R4000
	.set noreorder	
	mfc0	v0,C0_SR
	.set reorder	
	li	t0,SR_DE	
	bnez	a0,1f
	or	t1,v0,t0	# disable cache errors (SR_DE = 1)
	b	2f
1:	not	t1,t0		# enable cache errors (SR_DE = 0)
	and	t1,v0
	.set noreorder
2:	mtc0	t1,C0_SR
	.set reorder
	and	v0,t0		# get old SR_DE bit
	xor	v0,t0		# and invert to make it an enable bit
#endif
	j	ra
	.end	sbdparityenb


	.globl	wbflush
	.ent	wbflush
wbflush:
	WBFLUSH
	j	ra
	.end	wbflush


#define VACRST(reg,val) \
	.word	(reg), (val)

	.rdata
vacresettab:
    /*	
     * set the DRAM upper limit for local memory
     */
    VACRST(VACDRAMUL,		LOCAL_MEM_SIZE-1)

    /*
     * region 1 0x10000000:0x1effffff VME A32D32
     */
    VACRST(VACBOUND2,		LOCAL_PROM)
    VACRST(VACR1ATTR,		VAC_ATTR_MWB|VAC_ATTR_A32D32)
    
    /*
     * region 2 0x1f000000:0x1fffffff unused (inaccessible)
     */
    VACRST(VACBOUND3,		0x20000000)
    VACRST(VACR2ATTR,		VAC_ATTR_INACTIVE)
    
    /*
     * region 3 0x2000000:0xff000000 VME A32D32
     */
    VACRST(VACR3ATTR,		VAC_ATTR_MWB|VAC_ATTR_A32D32)
    
    /*
     * A24 overlay
     * 0x10000000:0x10ffffff VME A24D32
     * 0x11000000:0x11ffffff VME A24D32
     */
#ifdef VACA24BUG
    VACRST(VACA24,	 VAC_A24_ADDR(LOCAL_MEM_SIZE>>24)|VAC_A24_A24D32)
#else
    VACRST(VACA24,	 VAC_A24_ADDR(LOCAL_MEM_SIZE>>24))
#endif
    
    /*
     * VME address map
     */

    /*
     * SLSEL0: 0x1efe0000:1efeffff VMEA32D32
     */
    VACRST(VACSL0AMR,		0xffff)
    VACRST(VACSL0BAR,		0x1efe)
    VACRST(VIC_SEL0CR0,		VIC_SLD32|VIC_SLA32|VIC_SLSUP)
    VACRST(VIC_SEL0CR1,		VIC_SLDELINI(0) | VIC_SLDELSUB(0))

    /*
     * SLSEL1: 0x1eff0000:1effffff VMEA32D32
     */
    VACRST(VACSL1AMR,		0xffff)
    VACRST(VACSL1BAR,		0x1eff)
    VACRST(VIC_SEL1CR0,		VIC_SLD32|VIC_SLA32|VIC_SLSUP)
    VACRST(VIC_SEL1CR1,		VIC_SLDELINI(0) | VIC_SLDELSUB(0))

    VACRST(VACICFAR,		0)
    
    /* SCSI I/O parameters */
    VACRST(VACIOSEL0,	 	VAC_CS_PAS(0)| VAC_CS_REC(0)| VAC_CS_RDDEL(3)| VAC_CS_WRDEL(3)| VAC_CS_SLDEL(0)| VAC_CS_RDPAS| VAC_CS_WRPAS)

     /* SONIC I/O Parameters */
    VACRST(VACIOSEL1,	 	VAC_CS_PAS(0)| VAC_CS_REC(7)| VAC_CS_RDDEL(3)| VAC_CS_WRDEL(3)| VAC_CS_SLDEL(3)| VAC_CS_RDPAS| VAC_CS_WRPAS)

   /*
    * Miscellaneous devices I/O parameters
    */
    VACRST(VACIOSEL2,	 	VAC_CS_PAS(4)| VAC_CS_DSACKi0| VAC_CS_DSACKi1| VAC_CS_REC(7)| VAC_CS_RDDEL(3)| VAC_CS_WRDEL(3)| VAC_CS_SLDEL(0)| VAC_CS_RDPAS| VAC_CS_WRDSACK)
	
    /*
     * RTC/NVRAM I/O parameters
     */
    VACRST(VACIOSEL3,	 	VAC_CS_PAS(7)| VAC_CS_DSACKi0| VAC_CS_DSACKi1| VAC_CS_REC(7)| VAC_CS_RDDEL(3)| VAC_CS_WRDEL(3)| VAC_CS_SLDEL(3)| VAC_CS_RDPAS| VAC_CS_WRPAS)
	 
    /*
     * IOSEL4 not used
     */
    VACRST(VACIOSEL4,		0)

    /*
     * IOSEL5 not used
     */
    VACRST(VACIOSEL5,		0)

    /*
     * SHRCS not used
     */
    VACRST(VACSHRCS,		0)

    VACRST(VACDECODE,  		VAC_D_DRPAS|VAC_D_S1DRAMCS|VAC_D_A32A24|VAC_D_S0AS|VAC_D_S1AS|VAC_D_ICFAS|VAC_D_BQUAL|VAC_D_S1RED|VAC_D_S0RED|VAC_D_DDEL(0)|VAC_D_FPUPAS)

  /*
   * Route external and internal interrupt inputs
   * on to appropriate interrupt outputs
   */
    VACRST(VACICR,		VAC_MAPDEBUG(VAC_ICR_PIO11)|VAC_MAPPERR(VAC_ICR_PIO11)|VAC_MAPPIO7(VAC_ICR_NONE)|VAC_MAPPIO4(VAC_ICR_NONE)|VAC_MAPMAIL(VAC_ICR_PIO10)|VAC_MAPSIOA(VAC_ICR_PIO7)|VAC_MAPSIOB(VAC_ICR_PIO7)|VAC_MAPTIMER(VAC_ICR_PIO10))

    /*
     * No devices use the isolated bus
     */
    VACRST(VACDLOC,		0)

    /*
     * Define the PIO pin functions
     */
    VACRST(VACPIOF,		VAC_PIOF_CPUSPMB | VAC_PIOF_DEBEN | VAC_PIOF_IOSEL2 | VAC_PIOF_PIO11 | VAC_PIOF_PIO10 | VAC_PIOF_PIO7 | VAC_PIOF_IOSEL3 | VAC_PIOF_IOWR | VAC_PIOF_IORD | VAC_PIOF_RXDB | VAC_PIOF_TXDB | VAC_PIOF_RXDA | VAC_PIOF_TXDA)

    /* uart control */
    VACRST(VACCLKDIV,		VAC_CLK_16MHZ)
    VACRST(VACMRA,		VAC_UM_RXCEN|VAC_UM_TXCEN|VAC_UM_RXEN|VAC_UM_TXEN|VAC_UM_B9600)
    VACRST(VACMRB,		VAC_UM_RXCEN|VAC_UM_TXCEN|VAC_UM_RXEN|VAC_UM_TXEN|VAC_UM_B9600)
			     
    /* uart interupt mask (mask all) */
    VACRST(VACIMA,		0)
    VACRST(VACIMB,		0)

    /* this, at last, presses the VAC enable button !! */
    VACRST(VACID,		0)

    VACRST(0,			0)	/* terminator */
