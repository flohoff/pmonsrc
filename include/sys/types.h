/*
 * types.h : SDE system type definitions
 *
 * Copyright (c) 1993-1999, Algorithmics Ltd.  All rights reserved.
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

#ifndef __SYS_TYPES_H
#ifdef __cplusplus
extern "C" {
#endif
#define __SYS_TYPES_H

#include <mips/ansi.h>
#include <mips/types.h>

/* --- Types --- */
typedef unsigned short	uid_t;
typedef unsigned short	gid_t;

typedef short 		pid_t;
typedef pid_t 		pgrp_t;		/* what is this doing here */

typedef unsigned short 	dev_t;
typedef unsigned short 	ino_t;
typedef short 		nlink_t;
typedef long 	 	off_t;		/* limits file size to 2GB */
typedef unsigned short	mode_t;

typedef long 		ssize_t;

#ifdef _SIZE_T_		/* also in stddef.h */
typedef _SIZE_T_ size_t;
#undef _SIZE_T_
#endif

#ifdef __cplusplus
}
#endif
#endif /* !__SYS_TYPES_H */
