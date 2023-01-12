#ident "$Header: /vol/cvsroot/pmon/pmon/cogent/nvram.h,v 1.1.1.1 1996/01/10 16:37:11 chris Exp $"
/*
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

/*
 * nvram.h: NVRAM layout definitions
 */

#ifndef _NVRAM_
#define _NVRAM_
/*
 * defining ALGCOMPAT provides backward compatibility
 * with Algorithmics derived PROM monitors
 */
#define ALGCOMPAT
#ifdef ALGCOMPAT
#define NVOFFSET	0		/* use all of NVRAM */
#else
#define NVOFFSET	1024		/* first 1Kb reserved for DECelx */
#endif

/* Offsets to reserved locations */
    					/* size description */
#define NVOFF_MAGIC	(NVOFFSET+0)	/* 2 magic value */
#define NVOFF_CSUM	(NVOFFSET+2)	/* 2 NVRAM environment checksum */
#define NVOFF_ENVSIZE	(NVOFFSET+4)	/* 2 size of 'environment' */
#define NVOFF_TEST	(NVOFFSET+5)	/* 1 cold start test byte */
#define NVOFF_ETHADDR	(NVOFFSET+6)	/* 6 decoded ethernet address */
#define NVOFF_STATE  	(NVOFFSET+12)   /* 1 state of NVRAM */
#define NVOFF_UNUSED	(NVOFFSET+13)	/* 0 current end of table */

/* values for NVOFF_STATE */
#define NVSTATE_OK		0
#define NVSTATE_UNINITIALISED	1
#define NVSTATE_BADCHECKSUM	2
#define NVSTATE_RECOVERABLE	3
#define NVSTATE_UNRECOVERABLE	4

#define NV_MAGIC	0xdeaf		/* nvram magic number */
#define NV_RESERVED	64		/* number of reserved bytes */

#ifdef ALGCOMPAT
/* ho hum... */
#undef NVOFF_ETHADDR
#define NVOFF_ETHADDR	(NVOFFSET+NV_RESERVED-6)
#endif

/* number of bytes available for environment */
#define ENV_BASE	(NVOFFSET+NV_RESERVED)
#define ENV_TOP		TD_NVRAM_SIZE
#define ENV_AVAIL	(ENV_TOP-ENV_BASE)
#endif /* _NVRAM_ */
