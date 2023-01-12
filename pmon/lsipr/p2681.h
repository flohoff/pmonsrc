/* $Id: p2681.h,v 1.3 1997/01/20 18:18:13 chris Exp $ */
#ifndef _P2681_
#define _P2681_

#ifdef __ASSEMBLER__

/* offsets for externally defined devinfo data structure */
#define	SIOBASE	0
#define	BRATE	4
#define	CURACR	6

#else

struct p2681info {
	ubyte *siobase;
	ubyte brate[2];
	ubyte curacr;
	} ;
#endif

#endif /* _P2681_ */
