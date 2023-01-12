/*
 * nvenv.h: generic Algo-style environment in r/w non-volatile memory.
 *
 * Copyright (c) 1997-1999, Algorithmics Ltd.  All rights reserved.
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

#ifndef _ENVIRON_H_
#define _ENVIRON_H_

/* default offsets of reserved locations */
#ifndef ENVOFF_MAGIC
#define ENVOFF_MAGIC		0	/* 2 magic value */
#define ENVOFF_CSUM		2	/* 2 environment checksum */
#define ENVOFF_SIZE		4	/* 2 size of environment */
#ifdef ENVOFF_P4000_COMPAT
/* Algorithmics P4000 (old layout) */
#define ENVOFF_ETHADDR		6	/*  6 decoded ethernet address */
#define ENVOFF_STATE  		12	/*  1 state of NVRAM */
#define ENVOFF_CENTURY		13	/*  1 century byte */
#define ENVOFF_TEST		14	/*  2 cold start test bytes */
#define ENVOFF_SPARE  		16	/* 48 spare bytes */
#define ENVOFF_BASE		64	/*  ? start of environment */
#else /* !ENVOFF_P4000_COMPAT */
/* Algorithmics P4032 (new layout) */
#define ENVOFF_STATE  		6	/* 1 state of environment */
#define ENVOFF_SPARE  		7	/* 1 spare byte */
#define ENVOFF_TEST  		8	/* 2 test word */
#define ENVOFF_BASE		10	/* ? start of environment proper */
#endif /* !ENVOFF_P4000_COMPAT */
#endif /* !ENVOFF_NAGIC */

/* value for ENVOFF_MAGIC */
#define ENV_MAGIC		0xdeaf

/* values for ENVOFF_STATE */
#define ENVSTATE_UNKNOWN	-1
#define ENVSTATE_OK		0
#define ENVSTATE_UNINITIALISED	1
#define ENVSTATE_RECOVERABLE	2
#define ENVSTATE_UNRECOVERABLE	3
#define ENVSTATE_FORCERESET	4

#endif /*_ENVIRON_H_*/
