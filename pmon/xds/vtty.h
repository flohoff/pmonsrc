/* $Id: vtty.h,v 1.2 1996/01/16 14:25:27 chris Exp $ */
/*
 * VME pseudo tty interface
 */
#define VTTYBUFSIZE	8192

typedef unsigned char Uchar;
typedef unsigned int  Uint32;

/*
 * This data structure describes the layout of the pseudo tty shared memory.
 */
typedef struct {
    /*
     * This union conatins the 'device' control registers
     * This area is written by bot processors, so the must both
     * access it uncached to make sure that lie writebacks
     * don't corrupt adjacent data
     */
    union {
	struct vttycsr {
	    Uint32	csr_wflags;		/* writer flags */
	    Uint32	csr_wptr;		/* writer offset  */
	    Uint32	csr_rflags;		/* reader flags */
	    Uint32	csr_rptr;		/* reader offset  */
	    Uint32	csr_irq;		/* interrupt level */
	    Uint32	csr_vec;		/* interrupt vector */
	} u_csr;
#define VTTYPADSIZE (8*4)
	Uchar	u_pad[VTTYPADSIZE];		/* to align data to cache boundary */
    } u;
#define VTTYDATASIZE (VTTYBUFSIZE-VTTYPADSIZE)
    Uchar	vtty_buf[VTTYDATASIZE];		/* data buffer */
} vttybuf;

#define vtty_csr	u.u_csr
#define vtty_wflags	u.u_csr.csr_wflags
#define vtty_wptr	u.u_csr.csr_wptr
#define vtty_rflags	u.u_csr.csr_rflags
#define vtty_rptr	u.u_csr.csr_rptr
#define vtty_irq	u.u_csr.csr_irq
#define vtty_vec	u.u_csr.csr_vec

typedef struct {
    vttybuf	xbuf;		/* buffer written by XDS */
    vttybuf	sbuf;		/* buffer written by Sparc */
} vttydev;

#define VTTYNEXT(i) (i == (VTTYDATASIZE-1)) ? i = 0 : ++(i)
#define VTTYEMPTY(v) ((v).vtty_wptr == ((v).vtty_rptr))
#define VTTYSPACE(r, w) (VTTYDATASIZE-abs((r)-(w)))

#define VTTY_RDR_READY	1

#define NVTTY	4
