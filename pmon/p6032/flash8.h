/*
 * p6032/flash8.h: onboard 8bit flash support for Algorithmics P-6032/P-6064
 *
 * Copyright (c) 2000-2001, Algorithmics Ltd.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the "Free MIPS" License Agreement, a copy of 
 * which is available at:
 *
 *  http://www.algor.co.uk/ftp/pub/doc/freemips-license.txt
 *
 * You may not, however, modify or remove any part of this copyright 
 * message if this program is redistributed or reused in whole or in
 * part.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * "Free MIPS" License for more details.  
 */

/* XXX Support all families */
#define FROM_FAMILY_ATMEL	1
#define FROM_FAMILY_INTEL	1
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
