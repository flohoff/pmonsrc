/*
 * stddef.h : ANSI stddef definitions
 * Copyright (c) 1992 Algorithmics Ltd.
 */
#ifndef __STDDEF_H
#ifdef __cplusplus
extern "C" {
#endif
#define __STDDEF_H

/* --- Constants --- */
#ifndef NULL
#ifdef __GNUG__
#define NULL	__null
#else
#define NULL	((void *) 0)
#endif
#endif

/* --- Types --- */
#include <mips/ansi.h>

#ifdef _SIZE_T_
typedef _SIZE_T_ size_t;
#undef _SIZE_T_
#endif

#ifdef _PTRDIFF_T_
typedef _PTRDIFF_T_ ptrdiff_t;
#undef _PTRDIFF_T_
#endif

#ifdef _WCHAR_T_
typedef _WCHAR_T_ wchar_t;
#undef _WCHAR_T_
#endif

/* --- Macros --- */
#ifndef offsetof
#define offsetof(type, member)	((size_t) (&(((type *) 0)->member)))
#endif

#ifdef __cplusplus
}
#endif
#endif /* !defined __STDDEF_H */
