/* $Id: swenv.h,v 1.2 1996/01/16 14:25:25 chris Exp $ */
#define NVSIZE		SRAM_ENV_SIZE
#define NVOFFSET	0

/* Offsets to reserved locations */
#define NVOFF_MAGIC		0	/* 2b magic value */
#define NVOFF_CSUM		2	/* 2b environment checksum */
#define NVOFF_ENVSIZE		4	/* 2b size of environment */
#define NVOFF_STATE  		6	/* 1b state of NVRAM */
#define NVOFF_SPARE  		7	/* 1b spare byte */
#define NVOFF_ENVBASE		8	/* ?b start of environment */
#define NVOFF_ENVTOP		NVSIZE

/* values for NVOFF_STATE */
#define NVSTATE_UNKNOWN		-1
#define NVSTATE_OK		0
#define NVSTATE_UNINITIALISED	1
#define NVSTATE_RECOVERABLE	2
#define NVSTATE_UNRECOVERABLE	3

#define NV_MAGIC		0xdeaf		/* nvram magic number */

/* number of bytes available for environment */
#define ENV_AVAIL		(NVOFF_ENVTOP - NVOFF_ENVBASE)

