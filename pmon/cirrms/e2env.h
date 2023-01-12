/* $Id: e2env.h,v 1.2 1996/01/16 14:24:22 chris Exp $ */
/*
 * e2env.h: E2PROM environment layout definitions
 */

#ifndef _E2ENV_
#define _E2ENV_

#define NVOFFSET	0

/* Offsets to reserved locations */
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

/* number of bytes available for environment */
#define ENV_BASE	(NVOFFSET+NV_RESERVED)
#define ENV_TOP		512
#define ENV_AVAIL	(ENV_TOP-ENV_BASE)

#endif /* _E2ENV_ */
