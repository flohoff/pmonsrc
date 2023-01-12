/* $Id: termio.h,v 1.5 1999/03/22 16:46:32 nigel Exp $ */
#ifndef _TERMIO_
#define _TERMIO_

#ifndef NETIO
#define NETIO
#endif

#define FILEOFFSET	200	/* all files start at fd=200 */

/* operation codes for ioctl */
#define TCGETA		1
#define TCSETAF		2
#define TCSETAW		3
#define SETINTR		4
#define SETSANE		5
#define FIONREAD	6
#define GETINTR		7
#define GETTERM		8
#define SETTERM		9
#define SETNCNE		10
#define CBREAK		11
#define TERMTYPE	12

/* iflags */
#define ISTRIP 		0x0020	/* strip 8th bit off chars */
#define ICRNL  		0x0040	/* map CR to NL */
#define IXON   		0x0400	/* enable output flow control */
#define IXANY  		0x0800	/* any char restarts after stop */
#define IXOFF  		0x1000	/* enable input flow control */
#define ICTS_OFLOW	0x2000	/* CTS flow control of output */
#define IRTS_IFLOW	0x4000	/* RTS flow control of input */

/* oflags */
#define ONLCR  0x0004

/* lflags */
#define ISIG   0x0001
#define ICANON 0x0002
#define ECHO   0x0008
#define ECHOE  0x0010

/* lflags continued */
/* XXX these should be in cflags, but we've not got the space
   since we expanded the baud rate! */
#define CSIZE		0x0300	/* character size mask */
#define     CS8		    0x0000	    /* 8 bits */
#define     CS7		    0x0100	    /* 7 bits */
#define     CS6		    0x0200	    /* 6 bits */
#define     CS5		    0x0300	    /* 5 bits */
#define CSTOPB		0x0400		/* send 2 stop bits */
#define PARENB		0x0800		/* parity enable */
#define PARODD		0x1000		/* odd parity, else even */

/* cflags */
#define CXBAUD	0x8000		/* extended baud rates */
#define CXBAUDDECODE(c) (((c) & ~CXBAUD) * 100)
#define CXBAUDENCODE(b) (CXBAUD | ((b) / 100))

#define	CBAUD	0000017
#define	B0	0
#define	B50	0000001	
#define	B75	0000002
#define	B110	0000003
#define	B134	0000004
#define	B150	0000005
#define	B200	0000006
#define	B300	0000007
#define	B600	0000010
#define	B1200	0000011
#define	B1800	0000012
#define	B2400	0000013
#define	B4800	0000014
#define	B9600	0000015
#define	B19200	0000016
#define	B38400	0000017

/* cc definitions */
#define VINTR	0
#define VERASE	2
#define VEOL	5
#define VEOL2	6
#define V_START	8
#define V_STOP	9

/* operation codes for device specific driver */
#define OP_INIT		1
#define OP_TX		2
#define OP_RX		3
#define OP_RXRDY	4
#define OP_TXRDY 	5
#define OP_BAUD		6
#define OP_RXSTOP	7
#define OP_FLUSH	8
#define OP_RESET	9
#define OP_OPEN		10
#define OP_CLOSE	11
#define OP_XBAUD	12
#define OP_TCSET	13

#define DEV_MAX		8
#define STDIN		0
#define STDOUT		1
#define STDERR		2

/* operation codes for ttctl */
#define TT_CM		1	/* cursor movement */
#define TT_CLR		2	/* clear screen */
#define TT_CUROFF	3	/* switch cursor off */
#define TT_CURON 	4	/* switch cursor on */

#include "mips.h"

#ifdef LANGUAGE_C
#define CNTRL(x) (x & 0x1f)

#define NCC 23
struct termio {
	unsigned short c_iflag;
	unsigned short c_oflag;
	unsigned short c_cflag;
	unsigned short c_lflag;
	unsigned char c_cc[NCC];
	};

#include "stdio.h"

typedef struct ConfigEntry {
	Addr devinfo;
	int chan;
	iFunc *handler;
	int rxqsize;
	int brate;
	} ConfigEntry;

#include "queue.h"

typedef struct DevEntry {
	int txoff;
	int qsize;
	Queue *rxq;
	Addr sio;
	int chan;
	int rxoff;
	iFunc *handler;
	jmp_buf *intr;
	char *tname;
	iFunc *tfunc;
	struct termio t;
	} DevEntry;

typedef struct File {
	int dev;
	short valid;
#ifdef NETIO
	short netfd;
#endif
	} File;

typedef struct Ramfile {
	char *name;
	int open;
	unsigned long base;
	unsigned long size;
	unsigned long posn;
	} Ramfile;

extern DevEntry DevTable[DEV_MAX];
extern File _file[OPEN_MAX];
extern Ramfile _mfile[];
extern int *curlst;

extern const char *parsettymode (const char *, struct termio *);

#endif /* LANGUAGE_C */

#endif /* _TERMIO_ */

