/* $Id: vic068.h,v 1.2 1996/01/16 14:24:14 chris Exp $ */
/*
 * algvme/vic068.h: VIC068 definitions
 *
 * Accessing the VIC registers as 32 bit words makes the code
 * endianess independent.
 */

/* convert VIC068 register number into physical address */
#ifdef __ASSEMBLER__
#define VICREG(reg)	+(PHYS_TO_K1(VIC_BASE+(reg)-3))
#else
#define VICREG(reg)	((volatile unsigned int *)PHYS_TO_K1(VIC_BASE+(reg)-3))
#endif

/*
 * VMEbus Interrupt Control Registers
 */

/* Masking and level encoding of VMEbus interrupt acknowledge interrupt */
#define VIC_VIICR	VICREG(0x03) /* VMEbus Interrupter Interrupt */

/* Masking and level encoding of VMEbus interrupts IRQ1* - IRQ7* */
#define VIC_VICR1	VICREG(0x07) /* VMEbus Interrupt Control 1 */
#define VIC_VICR2	VICREG(0x0b) /* VMEbus Interrupt Control 2 */
#define VIC_VICR3	VICREG(0x0f) /* VMEbus Interrupt Control 3 */
#define VIC_VICR4	VICREG(0x13) /* VMEbus Interrupt Control 4 */
#define VIC_VICR5	VICREG(0x17) /* VMEbus Interrupt Control 5 */
#define VIC_VICR6	VICREG(0x1b) /* VMEbus Interrupt Control 6 */
#define VIC_VICR7	VICREG(0x1f) /* VMEbus Interrupt Control 7 */

/* Masking and level encoding of DMA status interrupt */
#define VIC_DMAICR	VICREG(0x23) /* DMA Status Interrupt Control */

/* Masking level encoding and polarity of local interrupts LIRQ1* - LIRQ7* */
#define VIC_LICR1	VICREG(0x27) /* Local Interrupt Control 1 */
#define VIC_LICR2	VICREG(0x2b) /* Local Interrupt Control 2 */
#define VIC_LICR3	VICREG(0x2f) /* Local Interrupt Control 3 */
#define VIC_LICR4	VICREG(0x33) /* Local Interrupt Control 4 */
#define VIC_LICR5	VICREG(0x37) /* Local Interrupt Control 5 */
#define VIC_LICR6	VICREG(0x3b) /* Local Interrupt Control 6 */
#define VIC_LICR7	VICREG(0x3f) /* Local Interrupt Control 7 */

/*
 * Masking and level encoding of ICGS 
 * Interprocessor Communications Global Switch Interrupt
 */
#define VIC_ICGSICR	VICREG(0x43) /* ICGS Interrupt Control */

/*
 * Masking and level encoding of ICMS 
 * Interprocessor Communications Module Switch Interrupt
 */
#define VIC_ICMSICR	VICREG(0x47) /* ICMS Interrupt Control */

/* Masking and level encoding of error group interrupts */
#define VIC_EGRPICR	VICREG(0x4b) /* Error Group Interrupt Control */

/*
 * The VIC interrupt control registers share a field indicating
 * the priority at which they should interrupt
 */
#define VIC_IPL0	0x00	/* VIC Interrupt priority level 0 */
#define VIC_IPL1	0x01	/* VIC Interrupt priority level 1 */
#define VIC_IPL2	0x02	/* VIC Interrupt priority level 2 */
#define VIC_IPL3	0x03	/* VIC Interrupt priority level 3 */
#define VIC_IPL4	0x04	/* VIC Interrupt priority level 4 */
#define VIC_IPL5	0x05	/* VIC Interrupt priority level 5 */
#define VIC_IPL6	0x06	/* VIC Interrupt priority level 6 */
#define VIC_IPL7	0x07	/* VIC Interrupt priority level 7 */

/*
 * Registers VIC_VIICR, VIC_VICRn, VIC_DMAICR and VIC_LICRn
 * share an interrupt enable bit
 */
#define VIC_INT_EN	0x00	/* Interrupt enabled */
#define VIC_INT_DIS	0x80	/* Interrupt disabled */

/*
 * The local interrupt control registers VIC_LICRn have fields
 * indicating the interrupt type
 */
#define VIC_LACTIVEHI	0x40	/* Interrupt is active high on rising edge */
#define VIC_LACTIVELO	0x00	/* Interrupt is active low on falling edge */
#define VIC_LEDGE	0x20	/* Interrupt is edge sensitive */
#define VIC_LLEVEL	0x00	/* Interrupt is level sensitive */
#define VIC_LVEC	0x10	/* VIC will supply interrupt vector */
#define VIC_LIRQ	0x08	/* State of LIRQn pin (when read) */

/* The VIC_ICGSICR register has four separate interrupt enables */
#define VIC_ICGS3_EN	0x00	/* ICGS3 interrupt disabled */
#define VIC_ICGS2_EN	0x00	/* ICGS2 interrupt disabled */
#define VIC_ICGS1_EN	0x00	/* ICGS1 interrupt disabled */
#define VIC_ICGS0_EN	0x00	/* ICGS0 interrupt disabled */
#define VIC_ICGS3_DIS	0x80	/* ICGS3 interrupt disabled */
#define VIC_ICGS2_DIS	0x40	/* ICGS2 interrupt disabled */
#define VIC_ICGS1_DIS	0x20	/* ICGS1 interrupt disabled */
#define VIC_ICGS0_DIS	0x10	/* ICGS0 interrupt disabled */

/* The VIC_ICMSICR register has four separate interrupt enables */
#define VIC_ICMS3_EN	0x00	/* ICMS3 interrupt disabled */
#define VIC_ICMS2_EN	0x00	/* ICMS2 interrupt disabled */
#define VIC_ICMS1_EN	0x00	/* ICMS1 interrupt disabled */
#define VIC_ICMS0_EN	0x00	/* ICMS0 interrupt disabled */
#define VIC_ICMS3_DIS	0x80	/* ICMS3 interrupt disabled */
#define VIC_ICMS2_DIS	0x40	/* ICMS2 interrupt disabled */
#define VIC_ICMS1_DIS	0x20	/* ICMS1 interrupt disabled */
#define VIC_ICMS0_DIS	0x10	/* ICMS0 interrupt disabled */

/* The VIC_EGRPICR register has four separate interrupt enables */
#define VIC_ACFAIL_EN	0x00	/* VME ACFAIL* i/u enabled */
#define VIC_WPOST_EN	0x00	/* VME write post fail i/u enabled */
#define VIC_ARBF_EN	0x00	/* VME arbitration timeout i/u enabled */
#define VIC_SYSF_EN	0x00	/* VME SYSFAIL* i/u enabled */
#define VIC_ACFAIL_DIS	0x80	/* VME ACFAIL* i/u disabled */
#define VIC_WPOST_DIS	0x40	/* VME write post fail i/u disabled */
#define VIC_ARBF_DIS	0x20	/* VME arbitration timeout i/u disabled */
#define VIC_SYSF_DIS	0x10	/* VME SYSFAIL* i/u disabled */

/*
 * The ICGS, ICMS, local and error group interrupts have an associated
 * vector register that can be used to identify the interrupt source
 * during an interrupt acknowledge. The bottom several bits are predefined
 * and used to indicate the interrupt source. The upper bits are user
 * programmable.
 * These registers must be written to enable interrupt encoding.
 */
#define VIC_ICGSVEC	VICREG(0x4f) /* ICGS Interrupt Vector Base */
#define VIC_ICMSVEC	VICREG(0x53) /* ICMS Interrupt Vector Base */
#define VIC_LOCVEC	VICREG(0x57) /* Local Interrupt Vector Base */
#define VIC_EGRPVEC	VICREG(0x5b) /* Error Group Interrupt Vector Base */

#define VIC_ICGSUMASK	0xfc	/* mask for definable ICGS vector */
#define VIC_ICGS0VEC	0x00	/* Predefined vector for ICGS0 */
#define VIC_ICGS1VEC	0x01	/* Predefined vector for ICGS1 */
#define VIC_ICGS2VEC	0x02	/* Predefined vector for ICGS2 */
#define VIC_ICGS3VEC	0x03	/* Predefined vector for ICGS3 */

#define VIC_ICMSUMASK	0xfc	/* mask for definable ICMS vector */
#define VIC_ICMS0VEC	0x00	/* Predefined vector for ICMS0 */
#define VIC_ICMS1VEC	0x01	/* Predefined vector for ICMS1 */
#define VIC_ICMS2VEC	0x02	/* Predefined vector for ICMS2 */
#define VIC_ICMS3VEC	0x03	/* Predefined vector for ICMS3 */

#define VIC_LIRQUMASK	0xf8	/* mask for definable LIRQn vector */
#define VIC_LIRQ0VEC	0x00	/* Predefined vector for LIRQ0 */
#define VIC_LIRQ1VEC	0x01	/* Predefined vector for LIRQ1 */
#define VIC_LIRQ2VEC	0x02	/* Predefined vector for LIRQ2 */
#define VIC_LIRQ3VEC	0x03	/* Predefined vector for LIRQ3 */
#define VIC_LIRQ4VEC	0x00	/* Predefined vector for LIRQ4 */
#define VIC_LIRQ5VEC	0x01	/* Predefined vector for LIRQ5 */
#define VIC_LIRQ6VEC	0x02	/* Predefined vector for LIRQ6 */
#define VIC_LIRQ7VEC	0x03	/* Predefined vector for LIRQ7 */

#define VIC_EGRPUMASK	0xf8	/* mask for definable err group vector */
#define VIC_ACFAILVEC	0x00	/* Predefined vector for ACFAIL* */
#define VIC_WPOSTVEC	0x01	/* Predefined vector for write post fail */
#define VIC_ARBFVEC	0x02	/* Predefined vector for arb failure */
#define VIC_SYSFVEC	0x03	/* Predefined vector for SYSFAIL* */
#define VIC_VIIVEC	0x04	/* Predefined vector for VME interrupter */
#define VIC_DMAVEC	0x05	/* Predefined vector for DMA */

/*
 * The VIC068 supports Interprocessor Communications Facilities (ICF).
 * These facilities are accessible from the VMEbus when the ICFSEL* input
 * is asserted.  This input is controlled by the VAC068.  The VAC068
 * should be initialised to specify the VMEbus A16 addresses used to generate
 * ICFSEL*.  The VAC068 may be used to decode two separate ICF addresses
 * One address range is used for module specific addresses. The other
 * is used for group addresses allowing several boards to be addressed
 * at once. XXX
 */

/*
 * The VIC068 supports eight single bit ``switches'' for 
 * interprocessor communications.  These switches can be written by
 * external VMEbus masters.  Four of these switches (the 
 * Interprocessor Communication Module Switches) are used for communications
 * with the local module. The remaining switches (the Interprocessor
 * Communication Global Switches) are used for broadcast communications with 
 * other VIC based VME boards on the VMEbus.
 * When these switches are set from the VMEbus an interrupt may be generated
 */

#define VIC_ICSR	VICREG(0x5f) /* Interprocessor Communications Switch  */
#define VIC_ICMSMASK	0xf0	/* mask for ICMS switches */
#define VIC_ICGSMASK	0x0f	/* mask for ICGS switches */

/*
 * There are eight eight-bit interprocessor communications registers
 * capable of being read from the local bus or the VMEbus
 * Five of the registers are general purpose. The remaining registers
 * have specialised uses.
 */

#define VIC_ICR0	VICREG(0x63) /* Interprocessor Communications 0 */
#define VIC_ICR1	VICREG(0x67) /* Interprocessor Communications 1 */
#define VIC_ICR2	VICREG(0x6b) /* Interprocessor Communications 2 */
#define VIC_ICR3	VICREG(0x6f) /* Interprocessor Communications 3 */
#define VIC_ICR4	VICREG(0x73) /* Interprocessor Communications 4 */

#define VIC_ID		VICREG(0x77) /* VIC ID register */
#define VIC_VSTATUS	VICREG(0x7b) /* VMEbus status register */
#define VIC_VCONTROL	VICREG(0x7f) /* VMEbus control register */

/*
 * The VIC_ID register is read-only and returns the revison number of
 * the VIC068 silicon.
 */
#define VIC_REV1	0xf1	/* Base value for Revsion 1 silicon */

/*
 * The VIC_VSTATUS register holds status information for the VMEbus 
 * interface.
 *
 * Six bits of the VIC_VSTATUS register are used for a software status
 * that can be written by the local processor for other boards to examine.
 * There are three predefined values
 *
 * Bit 6 is set when HALT* or IRESET* is asserted or when a
 * reset and hold state is entered (see the VIC_VCONTROL register).
 * This bit can also be used to assert SYSFAIL*.
 *
 * Bit 7 when read from the VME bus shows whether IRESET* is asserted.
 * When read locally it reflects the state of ACFAIL*.
 */
#define VIC_STATMASK	0x3f	/* Mask for software status */
#define VIC_STATRESET	0x3f	/* VIC068 has had a system reset */
#define VIC_STATLRESET	0x3e	/* VIC068 is performing local reset */
#define VIC_STATHALT	0x3d	/* VIC068 has detected system halt */
#define VIC_RESETHALT	0x40	/* (r) VIC068 is reset and halt */
#define VIC_SYSFAIL	0x40	/* (w) assert SYSFAIL* */
#define VIC_ACFAIL	0x80	/* (local r) status of ACFAIL* input */
#define VIC_VME_IRESET	0x80	/* (VME r) VIC068 has IRESET* asserted */

/*
 * The VIC_VCONTROL register provides semaphore switches for each
 * of the five general purpose IPC registers ICR0-ICR4. In addition,
 * a VMEbus master status bit, a reset and hold state control bit
 * and a SYSFAIL* mask bit are available.
 */
#define VIC_ICR0SEM	0x01	/* ICR0 semaphore (set when ICR0 written) */
#define VIC_ICR1SEM	0x02	/* ICR1 semaphore (set when ICR1 written) */
#define VIC_ICR2SEM	0x04	/* ICR2 semaphore (set when ICR2 written) */
#define VIC_ICR3SEM	0x08	/* ICR3 semaphore (set when ICR3 written) */
#define VIC_ICR4SEM	0x10	/* ICR4 semaphore (set when ICR4 written) */
#define VIC_MASTER	0x20	/* set when VIC is VMEbus master */
#define VIC_RANDH	0x40	/* enter reset and hold state */
#define VIC_SYSFMASK	0x80	/* don't assert SYSFAIL* in reset/hold */

/*
 * When accessing the IPC facilities from the VMEbus, byte accesses with
 * an AM code of 29 or 2d should be used.  The VAC068 decodes the address
 * to a block of 256 registers.  The VIC068 further decodes the address to
 * access the individual registers.
 */

/* Byte offsets for VME accesses to IPC facilities */
#define VIC_VME_ICR0	0x01	/* VME access to ICR0 */
#define VIC_VME_ICR1	0x03	/* VME access to ICR1 */
#define VIC_VME_ICR2	0x05	/* VME access to ICR2 */
#define VIC_VME_ICR3	0x07	/* VME access to ICR3 */
#define VIC_VME_ICR4	0x09	/* VME access to ICR4 */
#define VIC_VME_ICR5	0x0b	/* VME access to ICR5 */
#define VIC_VME_ICR6	0x0d	/* VME access to ICR6 */
#define VIC_VME_ICR7	0x0f	/* VME access to ICR7 */

#define VIC_VME_CICGS0	0x10	/* VME clear ICGS0 */
#define VIC_VME_SICGS0	0x11	/* VME set ICGS0 */
#define VIC_VME_CICGS1	0x12	/* VME clear ICGS1 */
#define VIC_VME_SICGS1	0x13	/* VME set ICGS1 */
#define VIC_VME_CICGS2	0x14	/* VME clear ICGS2 */
#define VIC_VME_SICGS2	0x15	/* VME set ICGS2 */
#define VIC_VME_CICGS3	0x16	/* VME clear ICGS3 */
#define VIC_VME_SICGS3	0x17	/* VME set ICGS3 */

#define VIC_VME_CICMS0	0x20	/* VME clear ICMS0 */
#define VIC_VME_SICMS0	0x21	/* VME set ICMS0 */
#define VIC_VME_CICMS1	0x22	/* VME clear ICMS1 */
#define VIC_VME_SICMS1	0x23	/* VME set ICMS1 */
#define VIC_VME_CICMS2	0x24	/* VME clear ICMS2 */
#define VIC_VME_SICMS2	0x25	/* VME set ICMS2 */
#define VIC_VME_CICMS3	0x26	/* VME clear ICMS3 */
#define VIC_VME_SICMS3	0x27	/* VME set ICMS3 */

/*
 * The VIC068 can act as a VME interrupter.  An interrupt can be asserted
 * or rescinded by writing to the VIC_VIRS register.  When read the register
 * returns the status of VMEbus interrupt requests pending.  These bits
 * will be cleared when the corresponding VMEbus interrupts are acknowledged
 */
#define VIC_VIRS	VIC_REG(0x83) /* VMEbus Interrupt Request/Status */

#define VIC_IRQASSERT	0x01	/* Assert VMEbus interrupts */
#define VIC_IRQRESCIND	0x01	/* Rescind VMEbus interrupts */
#define VIC_IRQ1	0x02	/* VMEbus interrupt 1 */
#define VIC_IRQ2	0x04	/* VMEbus interrupt 2 */
#define VIC_IRQ3	0x08	/* VMEbus interrupt 3 */
#define VIC_IRQ4	0x10	/* VMEbus interrupt 4 */
#define VIC_IRQ5	0x20	/* VMEbus interrupt 5 */
#define VIC_IRQ6	0x40	/* VMEbus interrupt 6 */
#define VIC_IRQ7	0x80	/* VMEbus interrupt 7 */

/*
 * Seven eight-bit interrupt vectors are held for use during a
 * VMEbus interrupt acknowledge cycle.
 */
#define VIC_VVEC1	VICREG(0x87) /* VMEbus interrupt 1 vector */
#define VIC_VVEC2	VICREG(0x8b) /* VMEbus interrupt 2 vector */
#define VIC_VVEC3	VICREG(0x8f) /* VMEbus interrupt 3 vector */
#define VIC_VVEC4	VICREG(0x93) /* VMEbus interrupt 4 vector */
#define VIC_VVEC5	VICREG(0x97) /* VMEbus interrupt 5 vector */
#define VIC_VVEC6	VICREG(0x9b) /* VMEbus interrupt 6 vector */
#define VIC_VVEC7	VICREG(0x9f) /* VMEbus interrupt 7 vector */

/*
 * The VIC068 has timeouts for both the VMEbus and its local bus.
 */
#define VIC_TRANTIME	VICREG(0xa3)	/* Transfer Timeout */
#define VIC_VMEDEL(i)	(((i)&7)<<5)	/* VME bus timeout */
#define VIC_LOCDEL(i)	(((i)&7)<<2)	/* Local bus timeout */
#define VIC_RESCDTAK	0x02		/* Rescind DTACK before tristating */
#define VIC_LOCVME	0x01		/* Local t/o includes VME t/o */

/* Values defined for VIC_VMEDEL(n) and VIC_LOCDEL(n) */
#define VIC_DEL4us	0	/* 4us bus timeout */
#define VIC_DEL16us	1	/* 16us bus timeout */
#define VIC_DEL32us	2	/* 32us bus timeout */
#define VIC_DEL64us	3	/* 64us bus timeout */
#define VIC_DEL128us	4	/* 128us bus timeout */
#define VIC_DEL256us	5	/* 256us bus timeout */
#define VIC_DEL512us	6	/* 512us bus timeout */
#define VIC_DELINFINITE	7	/* infinite bus timeout */

/*
 * Local bus timing (PAS* and DS*) can be controlled by the local
 * bus timing register.
 * Times in this register are in 64MHz clock periods.
 */
#define VIC_LBT		VICREG(0xa7)	/* Local Bus Timing */
#define VIC_PASASS(i)	((i)&0x0f)	/* PAS* assert for (i+2) cycles */
#define VIC_DS1		0x00		/* DS* for 1 cycles */
#define VIC_DS2		0x10		/* DS* for 2 cycles */
#define VIC_PASREC(i)	(((i)&0x07)<<5)	/* PAS* recovery for (i+1) cycles */

/*
 * Selection of 256 boundary crossings on the VMEbus and local address during
 * DMA block transfer is under control of the VIC_BTD register
 */
#define VIC_BTD		VICREG(0xab) /* Block Transfer Definition */

#define VIC_DPADEN	0x01	/* Enable dual path (not implemented) */
#define VIC_AMSR	0x02	/* XXX */
#define VIC_VME256	0x04	/* Allow VMEbus DMA across 256 boundary */
#define VIC_LOC256	0x08	/* Allow local DMA across 256 boundary */

/*
 * The interface configuration register is used to control certain
 * system interfaces
 */
#define VIC_IC		VICREG(0xaf) /* Interface Configuration */
#define VIC_SCON	0x01	/* State of SCON pin */
#define VIC_TURBO	0x02	/* Shave VMEbus access timings */
#define VIC_METASTAB	0x04	/* Enable longer metastability delays */
#define VIC_DEDLK1	0x08	/* Use alternate deadlock signalling */
#define VIC_DEDLK2	0x10	/* Use alternate deadlock signalling */
#define VIC_IAC1	0x20	/* Indivisible access on VMEbus */
#define VIC_IAC2	0x40	/* Indivisible access on VMEbus */
#define VIC_IAC3	0x80	/* Indivisible access on VMEbus */

/*
 * The arbiter/requester has its own configuration register
 */
#define VIC_ARBREQ	VICREG(0xb3) /* Arbiter/Requester Configuration */
#define VIC_PRI		0x80	/* Use priority scheduling on VMEbus */
#define VIC_RRS		0x00	/* Use round robin scheduling on VMEbus */
#define VIC_BR3		0x60	/* Use BR3 for VMEbus requests */
#define VIC_BR2		0x40	/* Use BR2 for VMEbus requests */
#define VIC_BR1		0x20	/* Use BR1 for VMEbus requests */
#define VIC_BR0		0x00	/* Use BR0 for VMEbus requests */
#define VIC_DRAMREF	0x10	/* Enable DRAM refresh */


/*
 * values for VIC_FAIRNESS(i)
 * other values for i (1 - 14) give a fairness of i*2us
 */
#define VIC_UNFAIR	0	/* Fairness disabled */
#define VIC_NOFAIR	15	/* Timeout disabled */

/*
 * The VIC_AMS register is used to source user-defined Address Modifier (AM)
 * codes to the VMEbus or to validate slave access cycles
 */

#define VIC_AMS		VICREG(0xb7) /* Address Modifier Source */
#define VIC_AMMASK	0x3f	/* AM(5:0) */
#define VIC_AMVAGUE	0x40	/* match slaves only on AM(5:3) */
#define VIC_ANFC	0x80	/* source AM(2:0) from FC* inputs */

/*
 * The bus error status register reports various bus errors recognised
 * by the VIC068.  The bus error bits are cleared by writing 0 to
 * this register
 */

#define VIC_BES		VICREG(0xbb) /* Bus Error Status */

#define VIC_VMEMASTER	0x80	/* VIC is VME master */
#define VIC_LBERR	0x40	/* local bus error (LBERR*) signalled */
#define VIC_BERR	0x20	/* bus error (BERR*) signalled */
#define VIC_VMETIM	0x10	/* VMEbus timeout */
#define VIC_LBTIM	0x08	/* Local bus timeout */
#define VIC_SELF0	0x04	/* self access by SLSEL0* */
#define VIC_SELF1	0x02	/* self access by SLSEL1* */
#define VIC_LBVME	0x01	/* local bus timeout while accessing VMEbus */

#define VIC_DMASTAT	VICREG(0xbf) /* DMA Status */
#define VIC_DCROSS	0x40	/* DMA crossed 256 byte boundary XXX*/
#define VIC_ODBERR	0x10	/* bus error (BERR*) signalled */
#define VIC_OLBERR	0x08	/* local bus error (LBERR*) signalled */
#define VIC_DDBERR	0x04	/* bus error (BERR*) during DMA */
#define VIC_DLBERR	0x02	/* local bus error (LBERR*) during DMA */
#define VIC_DMAINPROG	0x01	/* interleaved DMA in progress */

/*
 * There are two control registers for each of the two slave selects
 */
#define VIC_SEL0CR0	VICREG(0xc3) /* Slave Select 0/Control Register 0 */
#define VIC_SEL0CR1	VICREG(0xc7) /* Slave Select 0/Control Register 1 */
#define VIC_SEL1CR0	VICREG(0xcb) /* Slave Select 1/Control Register 0 */
#define VIC_SEL1CR1	VICREG(0xcf) /* Slave Select 1/Control Register 1 */

/*
 * Control Register 0 provides configuration information for slave
 * accesses in response to a SLSELn input.
 */
#define VIC_SLSWP	0x80	/* Enable slave write posting */
#define VIC_SLMWP	0x40	/* Enable master write posting */
#define VIC_SLSUP	0x20	/* Restrict access to supervisor mode */
#define VIC_SLD32	0x10	/* Enable D32 slave data size */

#define VIC_SLUSER	0x0c	/* Use AM defined in VIC_AM register */
#define VIC_SLA16	0x08	/* Use A16 address modifier */
#define VIC_SLA24	0x04	/* Use A24 address modifier */
#define VIC_SLA32	0x00	/* Use A32 address modifier */

#define VIC_SLNOBT	0x00	/* Do not support slave block transfers */
#define VIC_SLEMLOC	0x01	/* Emulate non-block on local bus */
#define VIC_SLACCEL	0x02	/* Accelerated block transfer */

/*
 * These bits are redefined in VIC_SEL0CR0 to provide controls 
 * for the timer interrupt provided on LIRQ2
 */
#define VIC_SL0TNONE	0x00	/* Timer disabled */
#define VIC_SL0T50	0x40	/* 50Hz timer */
#define VIC_SL0T1000	0x80	/* 1000Hz timer */
#define VIC_SL0T100	0xc0	/* 100Hz timer */

/*
 * Control Register 1 provides programmable slave access delays
 * The delays are programmed in multiples of 64MHz clock period:
 * 0, 2, 2.5, 3, 3.5 ... 9.5
 */
#define VIC_SLDELINI(i)	((i)&0x0f)	/* Initial DSACKi to DTACK delay */
#define VIC_SLDELSUB(i)	(((i)&0x0f)<<4) /* Next DSACKi to DTACK delay */

/*
 * The release control register controls the VMEbus release behavior and
 * the burst block length for VMEbus block transfers
 */

#define	VIC_RCR		VICREG(0xd3) /* Release Control Register */
#define VIC_BLENMASK	0x3f	/* Mask for VMEbus burst length */
#define	VIC_ROR		0x00	/* release on request mode */
#define	VIC_RWD		0x40	/* release when done */
#define	VIC_ROC		0x80	/* release on BCLR* */
#define	VIC_BCAP	0xc0	/* bus capture and hold */

/* 
 * The Block Transfer Control register determines the behavior of 
 * block transfer modes.
 */
#define VIC_BTC		VICREG(0xd7) /* Block Transfer Control Register */

/* 
 * The block transfer options are mutually exclusive.  Only one should 
 * be enabled at a time
 */
#define VIC_DMALOC	0x80	/* block transfer is DMA to local bus */
#define VIC_DMAVME	0x40	/* block transfer is DMA to VMEbus */
#define VIC_MOVEM	0x20	/* block transfer is MOVEM type */

#define VIC_DMAWR	0x10	/* Write DMA data to local bus */
#define VIC_DMARD	0x10	/* Read DMA data from local bus */

/* 
 * The local cycle interleave period between block transfer bursts is 
 * programmable. The delay period used is i*256ns
 */
#define VIC_BLTINT(i)	((i)&0x0f) /* BLT interleave period */

/*
 * The number of bytes to be transferred in a block transfer with 
 * local DMA can be programmed.  The VIC_BTLHI and VIC_BTLLO registers 
 * hold the 16 bit value representing the number of bytes to transfer. 
 * Note that the byte count must be even.
 */
#define VIC_BTLHI	VICREG(0xdb) /* Block Transfer Length Register 0 */
#define VIC_BTLLO	VICREG(0xdf) /* Block Transfer Length Register 1 */


/*
 * The VIC068 and the VMEbus may be reset by software.  Writing the value
 * VIC_RESETSYS to this register asserts SYSRESET* for 200ms and
 * resets all of the VIC068 internal registers
 */
#define VIC_SYSRESET	VICREG(0xe3) /* System Reset Register */

#define VIC_RESETSYS	0xf0	/* value to write to reset system */
