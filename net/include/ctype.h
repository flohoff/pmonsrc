/* $Id: ctype.h,v 1.2 1996/01/16 14:19:54 chris Exp $ */
#ifndef __CTYPE_H
#define __CTYPE_H

#include <sys/cdefs.h>

int isalpha	__P((int c));
int isupper	__P((int c));
int islower	__P((int c));
int isdigit	__P((int c));
int isspace	__P((int c));
int isalnum	__P((int c));
int isprint	__P((int c));
int iscntrl	__P((int c));
int ispunct	__P((int c));
int ishex	__P((int c));

/*int isgraph	__P((int c));*/
#ifdef IDTSIM
#define isxdigit(c)	ishex(c)
#endif
#define isascii(c)	((unsigned)(c) <= 0x7f)

int tolower	__P((int c));
int toupper	__P((int c));
int toascii	__P((int c));

/*int _tolower	__P((int c));*/
/*int _toupper	__P((int c));*/
#endif /* !CTYPE_H_ */
