/*
 *	
 *	Copyright (c) 1996 ALGORITHMICS LIMITED 
 *	ALL RIGHTS RESERVED 
 *	
 *	THIS SOFTWARE PRODUCT CONTAINS THE UNPUBLISHED SOURCE
 *	CODE OF ALGORITHMICS LIMITED
 *	
 *	The copyright notices above do not evidence any actual 
 *	or intended publication of such source code.
 *	
 */

/*
 * sbd.h: UBI Pablo laser printer rip - board information
 */

#define RAMCYCLE	80			/* 80ns dram */
#define ROMCYCLE	150			/* 150ns rom */

/* 
 * ROM/RAM and fixed memory address spaces etc.
 */
#define LOCAL_MEM	0x00000000 	/* Local on-board/private memory */
#define LOCAL_MEM_SIZE	0x02400000 	/* Local memory size (36Mb max) */
#define LOCAL_PROM	0x1f400000	/* PROM decode address */
#define LOCAL_PROM_SIZE	0x00800000	/* PROM decode size (12Mb max)*/
#define BOOTPROM	0x1fc00000	/* Boot Prom address */
#define BOOTPROM_SIZE	0x00400000	/* Boot Prom size (4Mb max)*/

#include <g10.h>

#ifdef MIPSEB
#define sbdreg8(x)		unsigned :24; unsigned char x;
#else
#define sbdreg8(x)		unsigned char x; unsigned :24; 
#endif

/* 2 x 16550 uarts on 8-bit bus D7:D0 */
#define NS16550_HZ		(24000000/13)
#define UART0_BASE		(IOCHAN0_BASE+(0x3f8*4))
#define UART1_BASE		(IOCHAN0_BASE+(0x2f8*4))
#define nsreg(x)		sbdreg8(x)

#define CSR_BASE		(IOCHAN1_BASE+0x00000)
#define CSR_COMBI_ENB		(CSR_BASE + 0x00)
#define CSR_ETH_ENB		(CSR_BASE + 0x04)
#define CSR_PRIF_ENB		(CSR_BASE + 0x08)
#define CSR_ROLL_ENB		(CSR_BASE + 0x0c)
#define CSR_PAGESYNC		(CSR_BASE + 0x10)
#define CSR_LINESYNC		(CSR_BASE + 0x14)
#define CSR_I2L_SCL		(CSR_BASE + 0x18)

#ifndef __ASSEMBLER__
struct csr {
    sbdreg8(csr_combi_enb);
    sbdreg8(csr_eth_enb);
    sbdreg8(csr_prif_enb);
    sbdreg8(csr_roll_enb);
    sbdreg8(csr_pagesync);
    sbdreg8(csr_linesync);
    sbdreg8(csr_i2l_scl);
};
#endif

/* Ethernet */
#define NIC_BASE		(IOGPCHAN0_BASE+0x200000)
#define FE_SYS_BUS_WIDTH	16
#define FE_DEBUG		3

/* disable loopback */
#define FE_D4_INIT		(FE_D4_CNTRL | FE_D4_LBC_DISABLE)

/* accept bad packets, promiscuous mode */
#define FE_D5_INIT		(FE_D5_BADPKT | FE_D5_AFM0 | FE_D5_AFM1)

/* force AUI interface, no link test */
#define FE_B13_INIT		(FE_B13_LNKTST | FE_B13_PORT_AUI)

/* since "accept bad packet" is set we mustn't enable error interrupts */
#define FE_RMASK 		(FE_D3_OVRFLO | FE_D3_BUSERR | FE_D3_PKTRDY)

/*
 * Program the 86964 as follows:
 *	SRAM: 32KB, 100ns, byte-wide access.
 *	Transmission buffer: 4KB x 2.
 *	System bus interface: 16 bits.
 */
#define FE_D6_INIT		(FE_D6_BUFSIZ_32KB | FE_D6_TXBSIZ_2x4KB | \
				 FE_D6_BBW_BYTE | FE_D6_SBW_WORD | FE_D6_SRAM_100ns)
#ifdef MIPSEB
#define FE_D7_INIT		(FE_D7_BYTSWP_HL | FE_D7_IDENT_EC)
#else
#define FE_D7_INIT		(FE_D7_BYTSWP_LH | FE_D7_IDENT_EC)
#endif

#define FPGAROM_BASE		IOGPCHAN1_BASE
#define FPGAROM_SIZE		32768

#define FPGA_BASE		ENGNCTL_BASE
#define FPGA_DMASK		0x0c	/* out: FPGA JTAG data mask */
#define FPGA_DSHFT 		2	/* out: FPGA JTAG data shift */
#define FPGA_TCK		0x02	/* out: FPGA JTAG clock */
#define FPGA_RST		0x01	/* out: FPGA reset */

#define G10_PIO_EE_D		0x01	/* i/o:	EEROM data */
#define G10_PIO_I2C_D		0x02	/* i/o:	I2C data */
#define G10_PIO_HSYNC		0x04	/* in:  printer HSYNC */
#define G10_PIO_ETHINT		0x08	/* in:	ethernet i/u */
#define G10_PIO_UARTINT0	0x10	/* in:  uart #0 i/u */
#define G10_PIO_UARTINT1	0x20	/* in:  uart #1 i/u */

#define INT_HSYNC		INT_PIO_2
#define INT_ETH			INT_PIO_3
#define INT_UART0		INT_PIO_4
#define INT_UART1		INT_PIO_5

#ifdef __ASSEMBLER__
#define WBFLUSH \
	.set nomove; \
        lw zero, PHYS_TO_K1(BOOTPROM); \
	.set move
#endif
