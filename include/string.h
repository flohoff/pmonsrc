/* $Id: string.h,v 1.2 1996/01/16 14:17:28 chris Exp $ */
#ifndef _STRING_
#define _STRING_
char *strcat(),*strncat(),*strchr(),*strncpy(),*cc2str(),*rindex();
char *strcpy(),*strichr(),*strdchr(),*strposn(),*getword(),*index();
char *strset(),*strrset(),*strrchr(),*strbalp(),*strrpset(),*strpbrk();
char *strtok();
int strequ(),strlequ(),strlen(),strpat();

/* definitions for fmt parameter of str_fmt(p,width,fmt) */
#define FMT_RJUST 0
#define FMT_LJUST 1
#define FMT_RJUST0 2
#define FMT_CENTER 3

#endif /* _STRING_ */
