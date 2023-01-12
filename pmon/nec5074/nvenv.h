/*
 * nvenv.h: Algo-style environment in r/w non-volatile memory.
 * Copyright (c) 1999  Algorithmics Ltd
 */

#ifndef _ENVIRON_H_
#define _ENVIRON_H_

#define ENVOFFSET	0		/* use all of NVRAM */

/* default offsets of reserved locations */
#ifndef ENVOFF_MAGIC
#define ENVOFF_MAGIC		0	/* 2 magic value */
#define ENVOFF_CSUM		2	/* 2 environment checksum */
#define ENVOFF_SIZE		4	/* 2 size of environment */
#define ENVOFF_TEST  		6	/* 2 test word */
#define ENVOFF_STATE  		12	/* 1 state of environment */
#define ENVOFF_CENTURY  	13	/* 1 real-time clock century */
#define ENVOFF_BASE		64	/* ? start of environment proper */
#endif

/* values for ENVOFF_MAGIC */
#define ENV_MAGIC		0xdeaf

/* values for ENVOFF_STATE */
#define ENVSTATE_UNKNOWN	-1
#define ENVSTATE_OK		0
#define ENVSTATE_UNINITIALISED	1
#define ENVSTATE_RECOVERABLE	2
#define ENVSTATE_UNRECOVERABLE	3
#define ENVSTATE_FORCERESET	4

#endif /*_ENVIRON_H_*/
