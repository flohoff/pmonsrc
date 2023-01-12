/*
 * flashenv.h: generic Algo-style environment in flash.
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
#endif
#ifndef ENVOFF_BASE
#define ENVOFF_BASE		2	/* ? start of environment proper */
#endif

/* values for ENVOFF_MAGIC */
#define ENV_MAGIC_0		0xde
#define ENV_MAGIC_1		0xaf

#define ENVSTATE_UNKNOWN	-1
#define ENVSTATE_OK		0
#define ENVSTATE_UNINITIALISED	1
#define ENVSTATE_RECOVERABLE	2
#define ENVSTATE_UNRECOVERABLE	3
#define ENVSTATE_MISSING	4
#define ENVSTATE_FORCERESET	5

#endif /*_ENVIRON_H_*/
