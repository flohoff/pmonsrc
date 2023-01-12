/*
 * stdarg.h : ANSI stdarg definitions
 * Copyright (c) 1992 Algorithmics Ltd.
 */
#ifndef __STDARG_H

#ifndef __need___va_list
#define __STDARG_H
#endif

#ifndef __VA_LIST
#define __VA_LIST
/* A generally safe declaration for stdio.h, without loading
   everything else and polluting the namespace */
typedef void *__va_list;
#endif

#ifndef __GNUC_VA_LIST
#define __GNUC_VA_LIST
/* For GNU C library */
typedef char *__gnuc_va_list;
#endif

#if defined(__need___va_list)
/* dummy only for stdio.h */
#elif	defined(__spur__)
#include "spur/sa-spur.h"
#elif	defined(__MIPSEB__) || defined(__MIPSEL__)
#include "mips/stdarg.h"
#elif	defined(__i860__)
#include "i860/sa-i860.h"
#elif	defined(__pyr__)
#include "pyr/sa-pyr.h"
#elif 	defined(__NeXT__)
#include "NeXT/sa-NeXT.h"
#elif 	defined(__sparc__)
#include "sparc/stdarg.h"
#elif 	defined(__mc68000__) || defined(__i386__) || defined(__ns32000__)
/* standard basic */

typedef char *va_list;

/* Amount of space required in an argument list for an arg of type TYPE.
   TYPE may alternatively be an expression whose type is used.  */

#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

#ifdef __VARARGS_H
/* old-style, non-ANSI compatibility */
#define va_alist __builtin_va_alist;
#define va_dcl   int __builtin_va_alist; ...

#define va_start(AP) \
  AP = (char *)&__builtin_va_alist;
#else
/* new-style ANSI */
#define va_start(AP, LASTARG) 						\
 (AP = ((char *) &(LASTARG) + __va_rounded_size (LASTARG)))
#endif

void va_end (va_list);		/* Defined in gnulib ?? */
#define va_end(AP)

#define va_arg(AP, TYPE)						\
 (AP += __va_rounded_size (TYPE),					\
  *((TYPE *) (AP - __va_rounded_size (TYPE))))

#else
#error "stdarg.h"
#endif

#undef __need___va_list

#endif /* !defined __STDARG_H */
