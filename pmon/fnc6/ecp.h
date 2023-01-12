/* 
 * ecp.h: ISA Extended Capabilities Port (ex. centronics) interface.
 */

#ifndef __ASSEMBLER__
typedef struct {
    unsigned int	data;	/* compatibility mode data */
#define ecpAFifo	data	/* ecp mode address fifo */
    unsigned int	dsr;	/* status register */
    unsigned int	dcr;	/* control register */
    unsigned int	gap[0x400-3];
    unsigned int	cFifo;	/* compatibility mode fifo */
#define ecpDFifo	cFifo	/* ecp mode data fifo */
#define tFifo		cFifo	/* test mode fifo */
#define cnfgA		cFifo	/* configuration mode reg A */
    unsigned int	cnfgB;	/* configuration mode reg B */
    unsigned int	ecr;	/* extended control register */
} ecpdev;
#endif

/* register offsets */
#define ECP_AFIFO	(0x000<<2)	/* ecp mode address fifo */
#define ECP_CDATA	(0x000<<2)	/* compatibility mode data */
#define ECP_DSR		(0x001<<2)	/* status register */
#define ECP_DCR		(0x002<<2)	/* control register */
#define ECP_DFIFO	(0x400<<2)	/* ecp mode data fifo */
#define ECP_CFIFO	(0x400<<2)	/* compatibility mode fifo */
#define ECP_TFIFO	(0x400<<2)	/* test mode fifo */
#define ECP_CNFGA	(0x400<<2)	/* configuration mode reg A */
#define ECP_CNFGB	(0x401<<2)	/* configuration mode reg B */
#define ECP_ECR		(0x402<<2)	/* extended control register */

/* status register (compatibility mode) */
#define DSR_NBUSY		0x80
#define DSR_NACK		0x40
#define DSR_PERROR		0x20
#define DSR_SELECT		0x10
#define DSR_NFAULT		0x08

/* status register (ECP mode) */
#define DSR_nPeriphAck	0x80
#define DSR_PeriphClk		0x40
#define DSR_nAckReverse	0x20
#define DSR_Xflag		0x10
#define DSR_nPeriphRequest	0x08

/* control register: common */
#define DCR_INPUT		0x20	/* port direction (1=input) */
#define DCR_INTENB		0x10	/* i/u enable on ACK (or STROBE) */

/* control register (compatibility mode) */
#define DCR_SELECTIN		0x08
#define DCR_NINIT		0x04
#define DCR_AUTOFD		0x02
#define DCR_STROBE		0x01

/* control register (ECP mode) */
#define DCR_nECPmode		0x08
#define DCR_nReverseRequest	0x04
#define DCR_nHostAck		0x02
#define DCR_nHostClk		0x01

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


