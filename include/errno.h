/* $Id: errno.h,v 1.4 1999/07/02 12:51:04 chris Exp $ */
#ifndef _ERRNO_
#define _ERRNO_

extern int errno;

#define EINVAL		1	/* invalid argument */
#define ENODEV		2	/* no such device */
#define ENOENT		2	/* No such file or directory */
#define EMFILE		3	/* too many open files */
#define EBADF		9	/* bad file descriptor */
#define ENOSPC		28	/* No space left on device */
#define	ERANGE		34	/* Result too large */
#define ENAMETOOLONG	63	/* File name too long */

#endif /* _ERRNO_ */
