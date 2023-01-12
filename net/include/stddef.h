/* $Id: stddef.h,v 1.2 1996/01/16 14:19:57 chris Exp $ */
/*
 * stddef.h : ANSI stddef definitions
 * Copyright (c) 1992 Algorithmics Ltd.
 */

#ifndef __STDDEF_H
#define __STDDEF_H

/* --- Constants --- */
#ifndef NULL
#define NULL	((void *) 0)
#endif

/* --- Types --- */
#include <machine/ansi.h>

#ifdef _SIZE_T_
typedef _SIZE_T_ size_t;
#undef _SIZE_T_
#endif

#ifdef _PTRDIFF_T_
typedef _PTRDIFF_T_ ptrdiff_t;
#undef _PTRDIFF_T_
#endif

/* --- Macros --- */
#ifndef offsetof
#define offsetof(type, member)	((size_t) (&(((type *) 0)->member)))
#endif

#endif /* !defined __STDDEF_H */
