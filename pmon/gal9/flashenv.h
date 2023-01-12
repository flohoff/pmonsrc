/*
 * environ.h: generic Algo-style environment in flash.
 * Copyright (c) 1997  Algorithmics Ltd
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

#endif /*_ENVIRON_H_*/
