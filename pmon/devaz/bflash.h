/*
 * devaz/bflash.h: onboard boot flash support for Algorithmics DEVA-0
 * Copyright (c) 1999 Algorithmics Ltd.
 */

/* Support only simple AMD parts */
#define FROM_FAMILY_AMD		1

/* Fixed device geometry on this board */
#define FROM_WIDTH		1	/* dev width = byte */
#define FROM_NCOLS		1	/* 1 devices in parallel */
#define FROM_GROUP		1	/* 1 device per access */

/* "Variable" device geometry */
/*#define FROM_SECSIZE(fcp)*/
/*#define FROM_DEVSIZE(fcp)*/
/*#define FROM_DEVSIZE_LOG2(fcp)*/
#define FROM_NBANKS(fcp)	1	/* 1 sequential bank */

/* Define to enable debug messages */
/*#define FROM_DEBUG*/

#ifdef FROM_DEBUG
#define FROM_TRIGGER(msg,addr,want,got)	\
    do { \
	volatile unsigned int *trig = PA_TO_KVA1 (IDE0_BASE); \
	trig[0] = (unsigned int)(addr); \
	trig[1] = want; \
	trig[2] = got; \
    } while (0)
#endif

/* Define if code (in particular flash programming code) 
   will never executed be from the flash rom. */
/*#define FROM_NEVER_SELF_PROGRAM*/

/* The device can always be mapped in and accessed as normal
   memory. */
#define FROM_MAPPABLE	1

/* If required, any special twiddling of the read or programming 
   address (as a line or column byte offset) can be done here. */

#if #endian(big)

#define FROM_READ_CONTIGUOUS(flags) \
    (((flags) & FLASHDEV_MODE) == FLASHDEV_READ_CODE_EB)

#define fromrline(x,flags) \
    ((((flags) & FLASHDEV_MODE) == FLASHDEV_READ_CODE_EB) ? (x) : ((x) ^ 3))

#define frompline(x,flags) \
    ((((flags) & FLASHDEV_MODE) == FLASHDEV_PROG_CODE_EB) ? (x) : ((x) ^ 3))

#endif
		   
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
