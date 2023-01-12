/* $Id: if_sonic.h,v 1.2 1996/01/16 14:21:13 chris Exp $ */
/*
 *	if_sonic.h: National Semiconductor SONIC definitions
 *	
 *	Copyright (c) 1992 ALGORITHMICS LIMITED 
 *	ALL RIGHTS RESERVED 
 *	
 *	THIS SOFTWARE PRODUCT CONTAINS THE UNPUBLISHED SOURCE
 *	CODE OF ALGORITHMICS LIMITED
 *	
 *	The copyright notices above do not evidence any actual 
 *	or intended publication of such source code.
 *	
 */


/* The endianness support in this file assumes the P4000i model,
   whereby the Sonic always remains big-endian, and the glue
   hardware does no byte swapping, but does swap 32-bit words
   within 64-bit dwords when the CPU is little-endian. */

#ifndef SN_REG
#ifdef MIPSEB
#define SN_REG(x) \
	unsigned short	:16; unsigned short x
#else
#define SN_REG(x) \
	unsigned short	x; unsigned short :16
#endif
#endif

#ifndef SN_MEM
#define SN_MEM(x) SN_REG(x)
#endif

/*
 * Receive Resource Descriptor
 * This structure describes the buffers into which packets
 * will be received.  Note that more than one packet may be
 * packed into a single buffer if constraints permit.
 */
struct RXrsrc {
  SN_MEM(buff_ptrlo);		/* buffer address LO */
  SN_MEM(buff_ptrhi);		/* buffer address HI */
  SN_MEM(buff_wclo);		/* buffer size (16bit words) LO */ 
  SN_MEM(buff_wchi);		/* buffer size (16bit words) HI */
};


/*
 * Receive Descriptor
 * This structure holds information about packets received.
 */
struct RXpkt {
  SN_MEM(status);		/* receive status */
  SN_MEM(byte_count);		/* packet byte count (including FCS) */
  SN_MEM(pkt_ptrlo);		/* packet data LO (in RBA) */
  SN_MEM(pkt_ptrhi);		/* packet data HI (in RBA) */
  SN_MEM(seqno);		/* rba/pkt sequence numbers */
  SN_MEM(rlink);		/* link to next receive descriptor */
  SN_MEM(in_use);		/* packet available to SONIC */
};

#define SEQNO_RBA(seq) ((seq) >> 8)
#define SEQNO_PKT(seq) ((seq) & 0xff)

/*
 * Transmit Descriptor
 * This structure holds information about packets to be transmitted.
 */
#ifndef FRAGMAX
#define FRAGMAX	64		/* maximum number of fragments in a packet */
#endif

struct TXpkt {
  SN_MEM(status);		/* transmitted packet status */
  SN_MEM(config);		/* transmission configuration */
  SN_MEM(pkt_size);		/* entire packet size in bytes */
  SN_MEM(frag_count);		/* # fragments in packet */
  union frag {
    struct {
      SN_MEM(_frag_ptrlo);	/* pointer to packet fragment LO */
      SN_MEM(_frag_ptrhi);	/* pointer to packet fragment HI */
      SN_MEM(_frag_size);	/* fragment size */
    } u_frag;
    struct {
      SN_MEM(_tlink);		/* link to next transmit descriptor */
    } u_link;
  } frags[FRAGMAX];
  SN_MEM(_pad);			/* make frags[FRAGMAX].u_link.link valid! */
};

#define frag_ptrlo	u_frag._frag_ptrlo
#define frag_ptrhi	u_frag._frag_ptrhi
#define frag_size	u_frag._frag_size
#define tlink		u_link._tlink


#define EOL	0x0001		/* end of list marker for link fields */


#ifndef MAXCAM
#define MAXCAM	16		/* number of user entries in CAM */
#endif

struct CDA {
  struct {
    SN_MEM(cam_ep);		/* CAM Entry Pointer */
    SN_MEM(cam_ap0);		/* CAM Address Port 0 xx-xx-xx-xx-YY-YY */
    SN_MEM(cam_ap1);		/* CAM Address Port 1 xx-xx-YY-YY-xxxx */
    SN_MEM(cam_ap2);		/* CAM Address Port 2 YY-YY-xx-xx-xx-xx */
  } desc[MAXCAM];
  SN_MEM(enable);		/* mask enabling CAM entries */
};


/*
 * SONIC registers as seen by the processor
 */
struct sn_reg {
  SN_REG(s_cr);			/* 00: Command */
  SN_REG(s_dcr);		/* 01: Data Configuration */
  SN_REG(s_rcr);		/* 02: Receive Control */
  SN_REG(s_tcr);		/* 03: Transmit Control */
  SN_REG(s_imr);		/* 04: Interrupt Mask */
  SN_REG(s_isr);		/* 05: Interrupt Status */
  SN_REG(s_utda);		/* 06: Upper Transmit Descriptor Address */
  SN_REG(s_ctda);		/* 07: Current Transmit Descriptor Address */
  SN_REG(_s_tps);		/* 08* Transmit Packet Size */
  SN_REG(_s_tfc);		/* 09* Transmit Fragment Count */
  SN_REG(_s_tsa0);		/* 0a* Transmit Start Address 0 */
  SN_REG(_s_tsa1);		/* 0b* Transmit Start Address 1 */
  SN_REG(_s_tfs);		/* 0c* Transmit Fragment Size */
  SN_REG(s_urda);		/* 0d: Upper Receive Descriptor Address */
  SN_REG(s_crda);		/* 0e: Current Receive Descriptor Address */
  SN_REG(_s_crba0);		/* 0f* Current Receive Buffer Address 0 */
  SN_REG(_s_crba1);		/* 10* Current Receive Buffer Address 1 */
  SN_REG(_s_rbwc0);		/* 11* Remaining Buffer Word Count 0 */
  SN_REG(_s_rbwc1);		/* 12* Remaining Buffer Word Count 1 */
  SN_REG(s_eobc);		/* 13: End Of Buffer Word Count */
  SN_REG(s_urra);		/* 14: Upper Receive Resource Address */
  SN_REG(s_rsa);		/* 15: Resource Start Address */
  SN_REG(s_rea);		/* 16: Resource End Address */
  SN_REG(s_rrp);		/* 17: Resource Read Pointer */
  SN_REG(s_rwp);		/* 18: Resource Write Pointer */
  SN_REG(_s_trba0);		/* 19* Temporary Receive Buffer Address 0 */
  SN_REG(_s_trba1);		/* 1a* Temporary Receive Buffer Address 1 */
  SN_REG(_s_tbwc0);		/* 1b* Temporary Buffer Word Count 0 */
  SN_REG(_s_tbwc1);		/* 1c* Temporary Buffer Word Count 1 */
  SN_REG(_s_addr0);		/* 1d* Address Generator 0 */
  SN_REG(_s_addr1);		/* 1e* Address Generator 1 */
  SN_REG(_s_llfa);		/* 1f* Last Link Field Address */
  SN_REG(_s_ttda);		/* 20* Temp Transmit Descriptor Address */
  SN_REG(s_cep);		/* 21: CAM Entry Pointer */
  SN_REG(s_cap2);		/* 22: CAM Address Port 2 */
  SN_REG(s_cap1);		/* 23: CAM Address Port 1 */
  SN_REG(s_cap0);		/* 24: CAM Address Port 0 */
  SN_REG(s_ce);			/* 25: CAM Enable */
  SN_REG(s_cdp);		/* 26: CAM Descriptor Pointer */
  SN_REG(s_cdc);		/* 27: CAM Descriptor Count */
  SN_REG(s_sr);			/* 28: Silicon Revision */
  SN_REG(s_wt0);		/* 29: Watchdog Timer 0 */
  SN_REG(s_wt1);		/* 2a: Watchdog Timer 1 */
  SN_REG(s_rsc);		/* 2b: Receive Sequence Counter */
  SN_REG(s_crct);		/* 2c: CRC Error Tally */
  SN_REG(s_faet);		/* 2d: FAE Tally */
  SN_REG(s_mpt);		/* 2e: Missed Packet Tally */
  SN_REG(_s_mdt);		/* 2f* Maximum Deferral Timer */
  SN_REG(_s_rtc);		/* 30* Receive Test Control */
  SN_REG(_s_ttc);		/* 31* Transmit Test Control */
  SN_REG(_s_dtc);		/* 32* DMA Test Control */
  SN_REG(_s_cc0);		/* 33* CAM Comparison 0 */
  SN_REG(_s_cc1);		/* 34* CAM Comparison 1 */
  SN_REG(_s_cc2);		/* 35* CAM Comparison 2 */
  SN_REG(_s_cm);		/* 36* CAM Match */
  SN_REG(_s_resvd1);		/* 37* resvd */
  SN_REG(_s_resvd2);		/* 38* resvd */
  SN_REG(_s_rbc);		/* 39* Receiver Byte Count */
  SN_REG(_s_resvd3);		/* 3a* resvd */
  SN_REG(_s_tbo);		/* 3b* Transmitter Backoff Counter */
  SN_REG(_s_trc);  		/* 3c* Transmitter Random Counter */
  SN_REG(_s_tbm);		/* 3d* Transmitter Backoff Mask */
  SN_REG(_s_resvd4);		/* 3e* resvd */
  SN_REG(s_dcr2);		/* 3f* Data Configuration 2 */
};

/*
 * Register Interpretations
 */

/*
 * The command register is used for issuing commands to the SONIC.
 * With the exception of CR_RST, the bit is reset when the operation
 * completes.
 */
#define CR_LCAM		0x0200	/* load CAM with descriptor at s_cdp */
#define CR_RRRA		0x0100	/* read next RRA descriptor at s_rrp */
#define CR_RST		0x0080	/* software reset */
#define CR_ST		0x0020	/* start timer */
#define CR_STP		0x0010	/* stop timer */
#define CR_RXEN		0x0008	/* receiver enable */
#define CR_RXDIS	0x0004	/* receiver disable */
#define CR_TXP		0x0002	/* transmit packets */
#define CR_HTX		0x0001	/* halt transmission */

/*
 * The data configuration register establishes the SONIC's bus cycle
 * operation.  This register can only be accessed when the SONIC is in
 * reset mode (s_cr.CR_RST is set.)
 */
#define DCR_EXBUS	0x8000	/* extended bus mode */
#define DCR_LBR		0x2000	/* latched bus retry */
#define DCR_PO1		0x1000	/* programmable output 1 */
#define DCR_PO0		0x0800	/* programmable output 0 */
#define DCR_STERM	0x0400	/* synchronous termination */
#define DCR_USR1	0x0200	/* reflects USR1 input pin */
#define DCR_USR0	0x0100	/* reflects USR0 input pin */
#define DCR_WC1		0x0080	/* wait state control 1 */
#define DCR_WC0		0x0040	/* wait state control 0 */
#define DCR_DW		0x0020	/* data width select */
#define DCR_BMS		0x0010	/* DMA block mode select */
#define DCR_RFT1	0x0008	/* receive FIFO threshold control 1 */
#define DCR_RFT0	0x0004	/* receive FIFO threshold control 0 */
#define DCR_TFT1	0x0002	/* transmit FIFO threshold control 1 */
#define DCR_TFT0	0x0001	/* transmit FIFO threshold control 0 */

/* data configuration register aliases */
#define DCR_SYNC	DCR_STERM /* synchronous (memory cycle 2 clocks) */
#define DCR_ASYNC	0	  /* asynchronous (memory cycle 3 clocks) */

#define DCR_WAIT0	0		  /* 0 wait states added */
#define DCR_WAIT1	DCR_WC0		  /* 1 wait state added */
#define DCR_WAIT2	DCR_WC1		  /* 2 wait states added */
#define DCR_WAIT3	(DCR_WC1|DCR_WC0) /* 3 wait states added */

#define DCR_DW16	0	/* use 16-bit DMA accesses */
#define DCR_DW32	DCR_DW	/* use 32-bit DMA accesses */

#define DCR_DMAEF	0	/* DMA until TX/RX FIFO has emptied/filled */
#define DCR_DMABLOCK	DCR_BMS	/* DMA until RX/TX threshold crossed */

#define DCR_RFT4	0		/* receive threshold 4 bytes */
#define DCR_RFT8	DCR_RFT0	/* receive threshold 8 bytes */
#define DCR_RFT16	DCR_RFT1 	/* receive threshold 16 bytes */
#define DCR_RFT24	(DCR_RFT1|DCR_RFT0) /* receive threshold 24 bytes */

#define DCR_TFT4	0		/* transmit threshold 4 bytes */
#define DCR_TFT16	DCR_TFT0	/* transmit threshold 16 bytes */
#define DCR_TFT24	DCR_TFT1	/* transmit threshold 24 bytes */
#define DCR_TFT28	(DCR_TFT1|DCR_TFT0) /* transmit threshold 28 bytes */

#define DCR2_PH		0x10		/* program hold */
#define DCR2_LRDY	0x08		/* latched ready */
#define DCR2_PCM	0x04		/* packet compress when matched */
#define DCR2_PCNM	0x02		/* packet compress when not matched */
#define DCR2_RJCM	0x01		/* reject on cam match */

/*
 * The receive control register is used to filter incoming packets and
 * provides status information on packets received.
 * The contents of the register are copied into the RXpkt.status field
 * when a packet is received. RCR_MC - RCR_PRX are then reset.
 */
#define RCR_ERR		0x8000	/* accept packets with CRC errors */
#define RCR_RNT		0x4000	/* accept runt (length < 64) packets */
#define RCR_BRD		0x2000	/* accept broadcast packets */
#define RCR_PRO		0x1000	/* accept all physical address packets */
#define RCR_AMC		0x0800	/* accept all multicast packets */
#define RCR_LB1		0x0400	/* loopback control 1 */
#define RCR_LB0		0x0200	/* loopback control 0 */
#define RCR_MC		0x0100	/* multicast packet received */
#define RCR_BC		0x0080	/* broadcast packet received */
#define RCR_LPKT	0x0040	/* last packet in RBA (RBWC < EOBC) */
#define RCR_CRS		0x0020	/* carrier sense activity */
#define RCR_COL		0x0010	/* collision activity */
#define RCR_CRC		0x0008	/* CRC error */
#define RCR_FAE		0x0004	/* frame alignment error */
#define RCR_LBK		0x0002	/* loopback packet received */
#define RCR_PRX		0x0001	/* packet received without errors */

/* receiver control register aliases */
/* the loopback control bits provide the following options */
#define RCR_LBNONE	0		/* no loopback - normal operation */
#define RCR_LBMAC	RCR_LB0		/* MAC loopback */
#define RCR_LBENDEC	RCR_LB1		/* ENDEC loopback */
#define RCR_LBTRANS	(RCR_LB1|RCR_LB0) /* transceiver loopback */

/*
 * The transmit control register controls the SONIC's transmit operations.
 * TCR_PINT - TCR_EXDIS are loaded from the TXpkt.config field at the
 * start of transmission.  
 */
#define TCR_PINT	0x8000	/* interrupt when transmission starts */
#define TCR_POWC	0x4000	/* program out of window collision timer */
#define TCR_CRCI	0x2000	/* transmit packet without 4 byte FCS */
#define TCR_EXDIS	0x1000	/* disable excessive deferral timer */

/* The transmit status register is written to the TXpkt.status field */
#define TSR_NC		0xf800	/* number of collisions */
#define  TSR_NCSHFT	 11
#define TSR_EXD		0x0400	/* excessive deferrals occurred (>3.2ms) */
#define TSR_DEF		0x0200	/* deferred transmissions occurred */
#define TSR_NCRS	0x0100	/* carrier not present during transmission */
#define TSR_CRSL	0x0080	/* carrier lost during transmission */
#define TSR_EXC		0x0040	/* excessive collisions (>16) detected */
#define TSR_OWC		0x0020	/* out of window (bad) collision occurred */
#define TSR_PMB		0x0008	/* packet monitored bad - the tansmitted
				 * packet had a bad source address or CRC */
#define TSR_FU		0x0004	/* FIFO underrun (memory access failed) */
#define TSR_BCM		0x0002	/* byte count mismatch (TXpkt.pkt_size
				 * != sum(TXpkt.frag_size) */
#define TSR_PTX		0x0001	/* packet transmitted without errors */

/* transmit control register aliases */
#define TCR_OWCSFD	0        /* start after start of frame delimiter */
#define TCR_OWCPRE	TCR_POWC /* start after first bit of preamble */


/*
 * The interrupt mask register masks the interrupts that
 * are generated from the interrupt status register.
 * All reserved bits should be written with 0.
 */
#define IMR_BREN	0x4000	/* bus retry occurred enable */
#define IMR_HBLEN	0x2000	/* heartbeat lost enable */
#define IMR_LCDEN	0x1000	/* load CAM done interrupt enable */
#define IMR_PINTEN	0x0800	/* programmable interrupt enable */
#define IMR_PRXEN	0x0400	/* packet received enable */
#define IMR_PTXEN	0x0200	/* packet transmitted enable */
#define IMR_TXEREN	0x0100	/* transmit error enable */
#define IMR_TCEN	0x0080	/* timer complete enable */
#define IMR_RDEEN	0x0040	/* receive descriptors exhausted enable */
#define IMR_RBEEN	0x0020	/* receive buffers exhausted enable */
#define IMR_RBAEEN	0x0010	/* receive buffer area exceeded enable */
#define IMR_CRCEN	0x0008	/* CRC tally counter rollover enable */
#define IMR_FAEEN	0x0004	/* FAE tally counter rollover enable */
#define IMR_MPEN	0x0002	/* MP tally counter rollover enable */
#define IMR_RFOEN	0x0001	/* receive FIFO overrun enable */


/*
 * The interrupt status register indicates the source of an interrupt when
 * the INT pin goes active.  The interrupt is acknowledged by writing
 * the appropriate bit(s) in this register.
 */
#define ISR_BR		0x4000	/* bus retry occurred */
#define ISR_HBL		0x2000	/* CD heartbeat lost */
#define ISR_LCD		0x1000	/* load CAM command has completed */
#define ISR_PINT	0x0800	/* programmed interrupt from TXpkt.config */
#define ISR_PKTRX	0x0400	/* packet received */
#define ISR_TXDN	0x0200	/* no remaining packets to be transmitted */
#define ISR_TXER	0x0100	/* packet transmission caused error */
#define ISR_TC		0x0080	/* timer complete */
#define ISR_RDE		0x0040	/* receive descriptors exhausted */
#define ISR_RBE		0x0020	/* receive buffers exhausted */
#define ISR_RBAE	0x0010	/* receive buffer area exceeded */
#define ISR_CRC		0x0008	/* CRC tally counter rollover */
#define ISR_FAE		0x0004	/* FAE tally counter rollover */
#define ISR_MP		0x0002	/* MP tally counter rollover */
#define ISR_RFO		0x0001	/* receive FIFO overrun */

#define ISR_ERRS	(ISR_RDE|ISR_RBE|ISR_RBAE|ISR_RFO|ISR_HBL|ISR_CRC|\
			 ISR_FAE|ISR_MP|ISR_RFO)


