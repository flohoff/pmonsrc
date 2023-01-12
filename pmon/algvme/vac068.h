/* $Id: vac068.h,v 1.2 1996/01/16 14:24:12 chris Exp $ */
/*
 * algvme/vac068.h: VAC068 definitions
 *
 * These definitions are extracted from the preliminary VAC068 specifiaction
 * Fields that appear to differ from the documentaion are marked with an
 * asterisk
 */

/*
 * Reading the VAC registers as 32 bit words makes the code
 * endianess independent.
 */

/* convert VAC068 register number into a physical address */
#ifdef __ASSEMBLER__
#define VACREG(reg) +(PHYS_TO_K1(VAC_BASE+((reg)*256)))
#else
#define VACREG(reg) ((volatile unsigned int *)(PHYS_TO_K1(VAC_BASE+((reg)*256))))
#endif

/*
 * The slave select registers are used to define a region of
 * addresses that will assert the SLSEL[01] outputs of the VAC068.
 * These are used to allow VMEbus slave accesses to onboard addresses.
 *
 * Each region is defined by a base address register and an address
 * mask register.
 *
 * A bit set in the address mask register enables a comparison of the
 * corresponding address bit with the base address register.  When a
 * match occurs the output is asserted.
 * i.e.		(addr & amr) == (bar & amr)
 *
 * The address matching occurs on address bits (31:16) of the address.
 *
 * The slave 0 registers assert SLSEL0* for VMEbus accesses.
 * Local accesses assert DRAMCS. This allows VMEbus slave accesses to
 * local memory.
 * The slave 1 registers assert SLSEL1* for VMEbus accesses.
 * Local accesses assert EPROMCS, VSBSEL, SHRCS or DRAMCS.
 * The EPROMCS, DRAMCS XXX outputs are implemented on the board.
 * This allows VMEbus slave accesses to one of these devices (see
 * Decode Control register.)
 */
#define VACSL0AMR	VACREG(0x02) /* SLSEL0* Address Mask */
#define VACSL0BAR	VACREG(0x03) /* SLSEL0* Base Address */

#define VACSL1AMR	VACREG(0x00) /* SLSEL1* Address Mask */
#define VACSL1BAR 	VACREG(0x01) /* SLSEL1* Base Address */

/*
 * The ICFSEL* Address Register is used to select the VMEbus addresses that
 * will assert the VAC068 ICFSEL* output pin. This is used to indicate
 * VMEbus interprocessor communications.
 * The two halves of this register are independently compared with VME
 * address bits A(15:08). The top half of the register should be used
 * to select the group address and the bottom half to select the module
 * specific address.
 */
#define VACICFAR	VACREG(0x04) /* ICFSEL* Address Register */
#define VAC_ICFGRP(i)	(((i)&0xff)<<24) /* Group address for ICFAMR */
#define VAC_ICFMOD(i)	(((i)&0xff)<<16) /* Module address for ICFAMR */

/*
 * VAC address space is split into five main areas. These areas are
 * defined by three registers and some fixed addresses. Additional
 * "special"  decoding is performed for certain address ranges
 * determined by base and limit registers.
 *
 * Physical address 	Register	Use
 * 0xffffffff
 *  					VAC/VIC/local I/O address space
 * 0xff000000
 *					Region 3
 * 0x????????		Boundary 3 Register
 *					Region 2
 * 0x????????		Boundary 2 Register		
 *   					Region 1
 * 0x????????		DRAM Upper Limit Mask Register
 *    					DRAM
 * 0x00000000
 *
 * The contents of the DRAM upper limit register are inverted and then 
 * NANDed with the local address bit by bit. If any of the bits are low
 * DRAMCS is not asserted (unless the address is being redirected by
 * SLSEL0* or SLSEL1*)
 *
 * If the address is less than the Boundary 2 Address Register and neither
 * DRAM nor the A24 space are being accessed, a valid region 1 access is
 * made.
 * If the address is less than the Boundary 3 Address Register and neither
 * DRAM nor the A24 nor region 1 is being accessed, a valid region 2 access 
 * is made.
 */
 
#define VACDRAMUL	VACREG(0x05) /* DRAM Upper Limit Mask */
#define VACBOUND2	VACREG(0x06) /* Boundary 2 */
#define VACBOUND3	VACREG(0x07) /* Boundary 3 */

/*
 * Regions 1, 2 and 3 have attributes affecting the type of VMEbus
 * transaction performed in those areas.
 */

#define VACR1ATTR	VACREG(0x09) /* Region 1 Attribute */
#define VACR2ATTR	VACREG(0x0a) /* Region 2 Attribute */
#define VACR3ATTR	VACREG(0x0b) /* Region 3 Attribute */

#define VAC_ATTR_WORD	0x80000000	/* Assert WORD* on access */
#define VAC_ATTR_ASIZ1	0x40000000	/* Assert ASIZ1* on access */
#define VAC_ATTR_ASIZ0	0x20000000	/* Assert ASIZ0* on access */
#define VAC_ATTR_CACHIN	0x10000000	/* Assert CACHIN* on access */
#define VAC_ATTR_INACTIVE 0x00000000	/* Region is inactive */
#define VAC_ATTR_SHRCS 	0x04000000	/* Region asserts SHRCS* */
#define VAC_ATTR_VSBSEL 0x08000000	/* Region asserts VSBSEL* */
#define VAC_ATTR_MWB 	0x0c000000	/* Region asserts MWB* (VME access) */

/* Standard access combinations */
#define VAC_ATTR_A32D32	(VAC_ATTR_WORD|VAC_ATTR_ASIZ1)
#define VAC_ATTR_A32D16 (              VAC_ATTR_ASIZ1)
#define VAC_ATTR_A24D32 (VAC_ATTR_WORD)
#define VAC_ATTR_A24D16 (0)
#define VAC_ATTR_A16D32 (VAC_ATTR_WORD|VAC_ATTR_ASIZ0)
#define VAC_ATTR_A16D16 (              VAC_ATTR_ASIZ0)
#define VAC_ATTR_AUDD32 (VAC_ATTR_WORD|VAC_ATTR_ASIZ1|VAC_ATTR_SIZE0)
#define VAC_ATTR_AUDD16 (              VAC_ATTR_ASIZ1|VAC_ATTR_SIZE0)

/*
 * The A24 Space Base Address Register specifies the addresses that will
 * perform VMEbus A24 transactions.
 * The register defines two 16Mb regions allowing D32 and D16 type
 * accesses.  Address bits A(31..25) are compared with the top 7 bits of
 * this register. When an exact match occurs a VMEbus transaction with A24
 * access is performed.
 */
#define VACA24		VACREG(0x08) /* A24 Space Base Address */
#ifdef VACA24BUG
#define VAC_A24_ADDR(i) (((i)&0xff)<<24)	/* Address mask */
#else
#define VAC_A24_ADDR(i) (((i)&0xfe)<<24)	/* Address mask */
#endif
#define VAC_A24_CACHIN	0x00800000 /* Assert CACHINH on A24 accesses */

#ifdef VACA24BUG
/*
 * *Due to a bug in the VAC chip, the selection of A24D16 and A24D32
 * accesses is dependent on the setting of the following bit in the VACA24 
 * register rather than bit24 of the address.
 */
#define VAC_A24_A24D32	0x01000000	/* Generate A24D32 accesses  */
#endif

/*
 * The device chip select outputs EPROMCS*, SHRCS*, IOSEL0*-IOSEL5*
 * have registers defining their timing characteristics for the
 * corresponding DSACKi*, IORD* and IOWR*.  The times are in multiples of
 * CPUCLK (C) or CPUCLK/2 (C/2) cycles.
 */

#define VACIOSEL0	VACREG(0x10) /* IOSEL0* Control Register */
#define VACIOSEL1	VACREG(0x11) /* IOSEL1* Control Register */
#define VACIOSEL2	VACREG(0x12) /* IOSEL2* Control Register */
#define VACIOSEL3	VACREG(0x13) /* IOSEL3* Control Register */
#define VACIOSEL4	VACREG(0x0c) /* IOSEL4* Control Register */
#define VACIOSEL5	VACREG(0x0d) /* IOSEL5* Control Register */
#define VACSHRCS	VACREG(0x0e) /* SHRCS* Control Register */
#define VACEPROMCS	VACREG(0x0f) /* EPROMCS* Control Register */

#define VAC_CS_PAS(i)	(((i)&7)<<29)	/* PAS* to DSACKi* in CPUCLK cycles */
#define VAC_CS_DSACKi0	0x10000000	/* XXX */
#define VAC_CS_DSACKi1	0x08000000	/* XXX */
#define VAC_CS_REC(i)	(((i)&7)<<24)	/* IOSELi* recovery in C cycles */
#define VAC_CS_RDDEL(i)	(((i)&3)<<22)	/* PAS* to IORD* in C/2 cycles */
#define VAC_CS_WRDEL(i)	(((i)&3)<<20)	/* PAS* to IOWR* in C/2 cycles */
#define VAC_CS_SLDEL(i)	(((i)&3)<<18)	/* PAS* to IOSELi* in C/2 cycles */
#define VAC_CS_RDPAS	0x00000000	/* deassert IORD* with PAS* */
#define VAC_CS_RDDSACK	0x00020000	/* deassert IORD* after DSACKi* dly */
#define VAC_CS_WRPAS	0x00000000	/* deassert IOWR* with PAS* */
#define VAC_CS_WRDSACK	0x00010000	/* deassert IOWR* after DSACKi* dly */

/*
 * The Decode Control Register makes global decisions about how the 
 * various regions are decoded
 */

#define VACDECODE	VACREG(0x14) /* Decode Control */

#define VAC_D_DSASS	0x80000000 /* Assert DSACKi* on LAEN */
#define VAC_D_DSTRI	0x00000000 /* Tristate DSACKi* on LAEN */
#define VAC_D_DRPAS	0x40000000 /* Qualify DRAMCS* with PAS* */
#define VAC_D_S1EPROMCS	0x00000000 /* SLSEL1* selects EPROMCS* */
#define VAC_D_S1VSBSEL	0x10000000 /* SLSEL1* selects VSBSEL* */
#define VAC_D_S1SHRCS	0x20000000 /* SLSEL1* selects SHRCS* */
#define VAC_D_S1DRAMCS	0x30000000 /* SLSEL1* selects DRAMCS* */
#define VAC_D_A32A24	0x08000000 /* Compare VME A(31:16) to BAR(15:0) */
#define VAC_D_A16	0x04000000 /* Compare VME A(15:8) to BAR(15:8) */
#define VAC_D_S0AS	0x02000000 /* Qualify SLSEL0* with VME AS* */
#define VAC_D_S1AS	0x01000000 /* Qualify SLSEL1* with VME AS* */
#define VAC_D_ICFAS	0x00800000 /* Qualify ICFSEL* with VME AS* */
#define VAC_D_BQUAL	0x00400000 /* Qualify bndry decode with PAS* or DS* */
#define VAC_D_DRCPU	0x00200000 /* Ack CPU DRAM access as 32bit port XXX */
#define VAC_D_S1RED	0x00100000 /* Enable SLSEL1 redirection */
#define VAC_D_S0RED	0x00080000 /* Enable SLSEL0 redirection to DRAMCS* */
#define VAC_D_DDEL(i)	(((i)&3)<<17) /* DSACKi* assertion delay to DRAMCS* */
#define VAC_D_FPUPAS	0x00010000 /* FPUCS* asserted on PAS* */
#define VAC_D_FPUCLK	0x00000000 /* FPUCS* asserted on CPUCLK low */

/*
 * The status of VAC068 interrupts is given by the Interrupt Status Register
 */
#define VACISR	VACREG(0x15) /* Interrupt Status Register */

#define VAC_ISR_PIO9	0x80000000 /* PIO9 (debug) interrupt pending */
#define VAC_ISR_PIO8	0x40000000 /* PIO8 interrupt pending */
#define VAC_ISR_PIO7	0x20000000 /* PIO7 interrupt pending */
#define VAC_ISR_PIO4	0x10000000 /* PIO4 interrupt pending */
#define VAC_ISR_MAILBOX	0x08000000 /* Mailbox interrupt pending */
#define VAC_ISR_TIMER	0x04000000 /* Timer interrupt pending */
#define VAC_ISR_UARTA	0x02000000 /* UART A sioirq pending */
#define VAC_ISR_UARTB	0x01000000 /* UART B sioirq pending */

/* Aliases for Algorithmics VME boards */
#define VAC_ISR_DEBUG	VAC_ISR_PIO9
#define VAC_ISR_PERR	VAC_ISR_PIO8

/*
 * The Interrupt Control Register maps interrupt requests onto one
 * of three external outputs.
 */
#define VACICR	VACREG(0x16) /* Interrupt Status Register */

#define VAC_ICR_NONE	0x00	/* mapping disabled */
#define VAC_ICR_PIO7	0x01	/* map interrupt to PIO7 output */
#define VAC_ICR_PIO10	0x02	/* map interrupt to PIO10 output */
#define VAC_ICR_PIO11	0x03	/* map interrupt to PIO11 output */
#define VAC_MAP_MASK	0x03	/* mask  */

#define VAC_MAPPIO9(i)	(((i)&3)<<30)	/* map PIO9 interrupt */
#define VAC_MAPPIO8(i)	(((i)&3)<<28)	/* map PIO8 interrupt */
#define VAC_MAPPIO7(i)	(((i)&3)<<26)	/* map PIO7 interrupt */
#define VAC_MAPPIO4(i)	(((i)&3)<<24)	/* map PIO4 interrupt */
#define VAC_MAPMAIL(i)	(((i)&3)<<22)	/* map mailbox interrupt */
#define VAC_MAPSIOA(i)	(((i)&3)<<20)	/* map UART A interrupt */
#define VAC_MAPSIOB(i)	(((i)&3)<<18)	/* map UART B interrupt */
#define VAC_MAPTIMER(i)	(((i)&3)<<16)	/* map timer interrupt */

/* aliases Algorithmics VME boards */
#define VAC_MAPDEBUG(i)	VAC_MAPPIO9(i)	/* map debug interrupt */
#define VAC_MAPPERR(i)	VAC_MAPPIO8(i)	/* map parity error interrupt */


/*
 * Slow I/O devices may be placed on their own data bus. The Device
 * Location register indicates the devices attached to the isolated
 * data bus.
 */
#define VACDLOC	VACREG(0x17) /* Device Location Register */

#define VAC_DLOC_IO5	0x00200000	/* IOSEL5 on isolated data bus */
#define VAC_DLOC_IO4	0x00100000	/* IOSEL4 on isolated data bus */
#define VAC_DLOC_IO3	0x00080000	/* IOSEL3 on isolated data bus */
#define VAC_DLOC_IO2	0x00040000	/* IOSEL2 on isolated data bus */
#define VAC_DLOC_IO1	0x00020000	/* IOSEL1 on isolated data bus */
#define VAC_DLOC_IO0	0x00010000	/* IOSEL0 on isolated data bus */

/*
 * The VAC068 supports user definable I/O pins.  Some of these pins
 * are dual purpose.  The pins function is determined by the PIO Function
 * Register.  There are two additional bits in this register. One is used to
 * enable interrupt acknowledge emulation when accessing the IOSEL5 region.
 * The other is used to enable the debounce delay associated with the 
 * PIO9 input.
 */
#define VACPIOF		VACREG(0x1b) /* PIO Function Register */

#define VAC_PIOF_CPUSPMB 0x80000000 /* Emulate FCIACK on IOSEL5 access */
#define VAC_PIOF_DEBEN	0x40000000 /* Enable debounce delay on PIO9 */
#define VAC_PIOF_IOSEL2	0x20000000 /* Enable IOSEL2* output */
#define VAC_PIOF_SHRCS	0x10000000 /* Enable SHRCS* output */
#define VAC_PIOF_PIO11	0x08000000 /* Enable PIO11 as interrupt output */
#define VAC_PIOF_PIO10	0x04000000 /* Enable PIO10 as interrupt output */
#define VAC_PIOF_IOSEL5	0x02000000 /* Enable IOSEL5* output */
#define VAC_PIOF_IOSEL4	0x01000000 /* Enable IOSEL4* output */
#define VAC_PIOF_PIO7	0x00800000 /* Enable PIO7 as interrupt output */
#define VAC_PIOF_IOSEL3	0x00400000 /* Enable IOSEL3* output */
#define VAC_PIOF_IOWR	0x00200000 /* Enable IOWR* output */
#define VAC_PIOF_IORD	0x00100000 /* Enable IORD* output */
#define VAC_PIOF_RXDB	0x00080000 /* Enable UART B RxData input */
#define VAC_PIOF_TXDB	0x00040000 /* Enable UART B TxData output */
#define VAC_PIOF_RXDA	0x00020000 /* Enable UART A RxData intput */
#define VAC_PIOF_TXDA	0x00010000 /* Enable UART A TxData output */

/*
 * The PIO pins that are configured as outputs can be controlled 
 * by writing to the PIO Data Output Register.
 * The PIO Pin Register reflects the status of the PIO pins XXX.
 * The PIO Direction register determines which of the enable PIO pins
 * are outputs. There is an additional configuration bit in this register
 * used to enable interrupt acknowledge emulation.
 */
#define VACPIODATAO	VACREG(0x18) /* PIO Data Output Register */
#define VACPIOPIN	VACREG(0x19) /* PIO Pin Register */
#define VACPIODIR	VACREG(0x1a) /* PIO Direction Register */

/* Bit positions for PIO direction register */
#define VAC_PIO_IACK	0x40000000 /* Emulate interrupt acknowledge */

#define VAC_PIO_13	0x20000000 /* PIO13 input/output */
#define VAC_PIO_12	0x10000000 /* PIO12 input/output */
#define VAC_PIO_11	0x08000000 /* PIO11 input/output */
#define VAC_PIO_10	0x04000000 /* PIO10 input/output */
#define VAC_PIO_9	0x02000000 /* PIO9 input/output */
#define VAC_PIO_8	0x01000000 /* PIO8 input/output */
#define VAC_PIO_7	0x00800000 /* PIO7 input/output */
#define VAC_PIO_6	0x00400000 /* PIO6 input/output */
#define VAC_PIO_5	0x00200000 /* PIO5 input/output */
#define VAC_PIO_4	0x00100000 /* PIO4 input/output */
#define VAC_PIO_3	0x00080000 /* PIO3 input/output */
#define VAC_PIO_2	0x00040000 /* PIO2 input/output */
#define VAC_PIO_1	0x00020000 /* PIO1 input/output */
#define VAC_PIO_0	0x00010000 /* PIO0 input/output */


/*
 * The VAC068 provides an internal baud rate generator used by the
 * internal UARTs.  The BRG is initialised by writing a divisor into
 * the CPU Clock Divisor Register.  The value to be written depends on
 * the system clock.
 */
#define VACCLKDIV	VACREG(0x1c) /* CPU Clock Divisor Register */

#define VAC_CLK_16MHZ	(105<<24) /* Value for 16MHz system clock */
#define VAC_CLK_20MHZ	(131<<24) /* Value for 20MHz system clock */
#define VAC_CLK_25MHZ	(164<<24) /* Value for 25MHz system clock */
#define VAC_CLK_30MHZ	(196<<24) /* Value for 30MHz system clock */
#define VAC_CLK_33MHZ	(216<<24) /* Value for 33MHz system clock */

/*
 * Two identical asynchronous serial channels are provided by the VAC068.
 * Six registers are used to control the channels.
 */

/*
 * The UART mode register configures a channel
 */
#define VACMRA		VACREG(0x1d) /* UART Channel A Mode */
#define VACMRB		VACREG(0x1f) /* UART Channel B Mode */

#define VAC_UM_PAREN	0x80000000 /* *Enable parity generation/checking */
#define VAC_UM_PAREVEN	0x40000000 /* Use even parity (if enabled) */
#define VAC_UM_8BITS	0x20000000 /* 8 bit data */
#define VAC_UM_7BITS	0x00000000 /* 7 bit data */
#define VAC_UM_B37	0x00000000 /* *37.5 baud */
#define VAC_UM_B75	0x04000000 /* *75 baud */
#define VAC_UM_B150	0x08000000 /* *150 baud */
#define VAC_UM_B300	0x0c000000 /* *300 baud */
#define VAC_UM_B600	0x10000000 /* *600 baud */
#define VAC_UM_B1200	0x14000000 /* *1200 baud */
#define VAC_UM_B2400	0x18000000 /* *2400 baud */
#define VAC_UM_B9600	0x1c000000 /* 9600 baud */
#define VAC_UM_RXCEN	0x02000000 /* Enable character receiver XXX */
#define VAC_UM_TXCEN	0x01000000 /* Enable character transmitter XXX */
#define VAC_UM_RXEN	0x00800000 /* Enable receiver */
#define VAC_UM_TXEN	0x00400000 /* Enable transmitter */
#define VAC_UM_BREAK	0x00200000 /* Send break */
#define VAC_UM_LOOP	0x00100000 /* Enable local loopback */

/* The transmitter is double buffered. */
#define VACTXA		VACREG(0x1e) /* UART Channel A Transmit Data */
#define VACTXB		VACREG(0x22) /* UART Channel B Transmit Data */
#define VACCTOTX(c)	((c) << 24) /* Convert c ready for transmission */

/*
 * The receiver has a four deep FIFO holding received data and status
 * information
 */
#define VACRXA		VACREG(0x20) /* UART Channel A Receiver FIFO */
#define VACRXB		VACREG(0x21) /* UART Channel B Receiver FIFO */

#define VAC_URX_BREAK	0x04000000 /* break detected */
#define VAC_URX_FRAME	0x02000000	/* framing error */
#define VAC_URX_PARITY	0x01000000	/* parity error */
#define VAC_URX_DMASK	0x00ff0000	/* mask for received data */
#define VACRXTOC(i)	(((i)&VAC_URX_DMASK)>>16) /* Rx character conversion */

/*
 * Various conditions can cause the UARTs to generate interrupts.
 * These conditions are controlled by interrupt mask registers.
 */
#define VACIMA		VACREG(0x23) /* UART Channel A Int Mask */
#define VACIMB		VACREG(0x24) /* UART Channel B Int Mask */

#define VAC_EN_RXC	0x80000000 /* Interrupt on receive character */
#define VAC_EN_FIFOF	0x40000000 /* Interrupt on receive FIFO full */
#define VAC_EN_BRK	0x20000000 /* Interrupt on break */
#define VAC_EN_ERR	0x10000000 /* Interrupt on framing/parity error */
#define VAC_EN_TXRDY	0x08000000 /* Interrupt on transmitter ready */
#define VAC_EN_TXMTY	0x04000000 /* Interrupt on transmitter empty */

/*
 * The cause of an interrupt is determined by examining the interrupt
 * status register.
 */
#define VACISA		VACREG(0x25) /* UART Channel A Int Status */
#define VACISB		VACREG(0x26) /* UART Channel B Int Status */

#define VAC_UIS_RXC	0x80000000 /* Interrupted for received character */
#define VAC_UIS_FIFOF	0x40000000 /* Interrupted for receive FIFO full */
#define VAC_UIS_BRK	0x20000000 /* Interrupted for break status change */
#define VAC_UIS_PERR	0x10000000 /* Interrupted for parity error */
#define VAC_UIS_FERR	0x08000000 /* Interrupted for framing error */
#define VAC_UIS_OERR	0x04000000 /* Interrupted for overrun error */
#define VAC_UIS_TXE	0x02000000 /* *Interrupted for transmitter empty */
#define VAC_UIS_TXR	0x01000000 /* *Interrupted for transmitter ready */

  
/*
 * The programmable timer is capable of generating interrupts and measuring
 * small time intervals.  It can be used in a one-shot or continuous
 * mode.
 * The main clock input to the timer is the VAC068 CPUCLK input which runs at
 * 16MHz.  This clock is divided by the prescaler value.  The resulting
 * output is used to increment the timer register.  When the timer register
 * becomes zero an interrupt may be generated and the timer restarted.
 *
 * The prescaler count can be obtained by reading the timer control register.
 * The timer count can be obtained by reading the timer data register.
 * Note that the prescaler and timers both count up to zero.
 */
#define VACCLK		16000000

#define VACTC		VACREG(0x28) /* Timer Control */
#define VACTD		VACREG(0x27) /* Timer Data */

#define VAC_TC_ONCE	0x80000000 /* one-shot mode */
#define VAC_TC_CONT	0x00000000 /* continuous mode */
#define VAC_TC_ENABLE	0x40000000 /* enable timer */
#define VAC_TC_PRE(i)	(((i)&0x3f)<<24) /* set prescaler */

#define VAC_TC_PMASK	0x00ff0000 /* read mask for prescaler output */


/*
 * The Identification register returns the VAC068 identifier and a
 * silicon revision number.
 * To enable the VAC068 once all initialisation has been done, a
 * write must be performed to this register.
 */
#define VACID		VACREG(0x29) /* VAC068 Identification */

#define VAC_IDENT	0x1ac00000	/* VAC068 identifier */
#define VAC_REVMASK	0x000f0000	/* revision level mask */

