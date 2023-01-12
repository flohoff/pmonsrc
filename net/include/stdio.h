/* $Id: stdio.h,v 1.2 1996/01/16 14:19:58 chris Exp $ */
#ifndef IDTSIM

#include_next <stdio.h>

#define setbuf(f, n)	0
#define BUFSIZ		512

#else /* !IDTSIM */

#ifndef _STDIO_H_
#define _STDIO_H_

extern int stdin, stdout;
#define stderr stdout

#define EOF		(-1)
#ifndef NULL
#define NULL		((char *)0)
#endif

#define BUFSIZ		64	/* fake fake */

#define fflush(f)	0
#define setbuf(f, n)	0

#endif /* !_STDIO_H_ */
#endif /* !IDTSIM */
