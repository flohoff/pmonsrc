/*
 * CUDIG/sbdfrom.h: onboard FROM support for Telegate CUDIG
 * Copyright (c) 1999 Algorithmics Ltd.
 */

/* Fixed device geometry on this board */
#define FROM_WIDTH		2	/* dev width = word */
#define FROM_WIDTH_LOG2		1	/* log2(2) = 1 */
#define FROM_NCOLS		2	/* 2 devices in parallel */
#define FROM_NCOLS_LOG2		1	/* log2(ncols) */
#define FROM_GROUP		FROM_NCOLS
/*#define FROM_LINESZ*/
/*#define FROM_LINESZ_LOG2*/

/* "Variable" device geometry (fixed in this case) */
#define FROM_SECSIZE(fcp)	0x10000 /* 64K per sector */
#define FROM_DEVSIZE(fcp)	0x200000 /* 2MB per chip */
#define FROM_DEVSIZE_LOG2(fcp)	21	/* log2(devsize) */
#define FROM_NBANKS(fcp)	1	/* 1 sequential bank */

/* Define to enable debug messages */
/*#define FROM_DEBUG*/

/* Define if code (in particular flash programming code) 
   will never executed be from the flash rom. */
#define FROM_NEVER_SELF_PROGRAM

/* Define if this board can never have an Atmel sector-programming
   (implicit erase) device. */
#define FROM_NEVER_ATMEL

/* Define if this board can never have an Intel-style device */
#define FROM_NEVER_INTEL

/* Define if this board can never have an AMD-style device */
/*#define FROM_NEVER_AMD*/

/* If required, any special twiddling of the read or programming 
   address (as a from_t[] line or column index) can be done here. */
/*#define fromrline(x,flags)*/
/*#define frompline(x,flags)*/
/*#define fromrcol(x,flags)*/
/*#define frompcol(x,flags)*/

/* Enable/disable WR */
/*#define fromwrenable(fcp)*/
/*#define fromwrdisable(fcp)*/

/* Enable/Disable Vpp */
/*#define fromvppenable(fcp)*/
/*#define fromvppdisable(fcp)*/

/* Return h/w protect status for offset */
/*#define fromprotected(fcp, offs)*/
