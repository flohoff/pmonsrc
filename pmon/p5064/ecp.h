/* 
 * ecp.h: ISA Extended Capabilities Port (ex. centronics) interface on P4032.
 * Reversible to allow us to be a "peripheral" rather than a "host".
 */

#ifndef __ASSEMBLER__
typedef struct {
    unsigned char	data;	/* compatibility mode data */
#define ecpAFifo	data	/* ecp mode address fifo */
    unsigned char	dsr;	/* status register */
    unsigned char	dcr;	/* control register */
    unsigned char	dummy;	/* unused */
    unsigned char	latch;	/* P5064 input latch */
    unsigned char	gap[0x400-5];
    unsigned char	cFifo;	/* compatibility mode fifo */
#define ecpDFifo	cFifo	/* ecp mode data fifo */
#define tFifo		cFifo	/* test mode fifo */
#define cnfgA		cFifo	/* configuration mode reg A */
    unsigned char	cnfgB;	/* configuration mode reg B */
    unsigned char	ecr;	/* extended control register */
} ecpdev;
#endif

/* register offsets */
#define ECP_AFIFO	(0x000<<2)	/* ecp mode address fifo */
#define ECP_CDATA	(0x000<<2)	/* compatibility mode data */
#define ECP_DSR		(0x001<<2)	/* status register */
#define ECP_DCR		(0x002<<2)	/* control register */
#define ECP_LATCH	(0x003<<2)	/* P5064 input latch */
#define ECP_DFIFO	(0x400<<2)	/* ecp mode data fifo */
#define ECP_CFIFO	(0x400<<2)	/* compatibility mode fifo */
#define ECP_TFIFO	(0x400<<2)	/* test mode fifo */
#define ECP_CNFGA	(0x400<<2)	/* configuration mode reg A */
#define ECP_CNFGB	(0x401<<2)	/* configuration mode reg B */
#define ECP_ECR		(0x402<<2)	/* extended control register */

/* status register: as host (compatibility mode) */
#define DSR_H_NBUSY		0x80
#define DSR_H_NACK		0x40
#define DSR_H_PERROR		0x20
#define DSR_H_SELECT		0x10
#define DSR_H_NFAULT		0x08

/* status register: as peripheral (compatibility mode) */
#define DSR_P_AUTOFD		0x80
#define DSR_P_NSTROBE		0x40
#define DSR_P_NINIT		0x20
#define DSR_P_NSELECTIN		0x10

/* status register: as host (ECP mode) */
#define DSR_H_nPeriphAck	0x80
#define DSR_H_PeriphClk		0x40
#define DSR_H_nAckReverse	0x20
#define DSR_H_Xflag		0x10
#define DSR_H_nPeriphRequest	0x08

/* status register: as peripheral (ECP mode) */
#define DSR_P_nHostAck		0x80
#define DSR_P_HostClk		0x40
#define DSR_P_nReverseRequest	0x20
#define DSR_P_ECPmode		0x10

/* control register: common */
#define DCR_INPUT		0x20	/* port direction (1=input) */
#define DCR_INTENB		0x10	/* i/u enable on ACK (or STROBE) */

/* control register: as host (compatibility mode) */
#define DCR_H_SELECTIN		0x08
#define DCR_H_NINIT		0x04
#define DCR_H_AUTOFD		0x02
#define DCR_H_STROBE		0x01

/* control register: as peripheral (compatibility mode)  */
#define DCR_P_NSELECT		0x08
#define DCR_P_PERROR		0x04
#define DCR_P_NBUSY		0x02
#define DCR_P_ACK		0x01

/* control register: as host (ECP mode) */
#define DCR_H_nECPmode		0x08
#define DCR_H_nReverseRequest	0x04
#define DCR_H_nHostAck		0x02
#define DCR_H_nHostClk		0x01

/* control register: as peripheral (ECP mode)  */
#define DCR_P_Xflag		0x08
#define DCR_P_nAckReverse	0x04
#define DCR_P_nPeriphAck	0x02
#define DCR_P_nPeriphClk	0x01

/* extended control register */
#define ECR_MODE	0xe0	/* current mode: */
#define  ECR_MODE_COMPAT (0<<5)	 /* compatibility mode (host->periph only)  */
#define  ECR_MODE_BIDIR	 (1<<5)	 /* simple bidirectional mode */
#define  ECR_MODE_CFIFO	 (2<<5)	 /* compatibility mode (with output fifo) */
#define  ECR_MODE_ECP	 (3<<5)	 /* ECP mode */
#define  ECR_MODE_EPP	 (4<<5)	 /* EPP mode */
#define  ECR_MODE_TEST	 (6<<5)	 /* test mode */
#define  ECR_MODE_CNFG	 (7<<5)	 /* configuration mode */
#define ECR_ERRINTR_DIS 0x10	/* disable error interrupt on nFault */
#define ECR_DMA_EN	0x08	/* DMA enable */
#define ECR_INTR	0x04	/* DMA terminal count or FIFO i/u */
#define ECR_FIFO_FULL	0x02	/* FIFO is full */
#define ECR_FIFO_EMPTY	0x01	/* FIFO is empty */


