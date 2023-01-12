/*
 * BLM/sbdfrom.h: onboard FROM support for Siemens Atea BLM-RISC
 * Copyright (c) 1999 Algorithmics Ltd.
 */

/* Fixed device geometry on this board */
#define FROM_WIDTH		2	/* dev width = word */
#define FROM_NCOLS		2	/* 2 devices in parallel */
#define FROM_GROUP		2	/* program 2 in parallel */

/* "Variable" device geometry */
/*#define FROM_SECSIZE(fcp)*/
/*#define FROM_DEVSIZE(fcp)*/
/*#define FROM_DEVSIZE_LOG2(fcp)*/
/*#define FROM_NBANKS(fcp)*/

/* Define to enable debug messages */
/*#define FROM_DEBUG*/

/* Define if code (in particular flash programming code) 
   will never executed be from the flash rom. */
#define FROM_NEVER_SELF_PROGRAM

/* Define if this board can never have an Atmel sector-programming
   (implicit erase) device. */
#define FROM_NEVER_ATMEL

/* Define if this board can never have an Intel-style device */
/*#define FROM_NEVER_INTEL*/

/* Define if this board can never have an AMD-style device */
#define FROM_NEVER_AMD

/* If required, any special twiddling of the read or programming 
   address (as a line or column byte offset) can be done here. */
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
