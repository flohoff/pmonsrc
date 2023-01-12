/* $Id: signal.c,v 1.2 1996/01/16 14:18:27 chris Exp $ */
#include <signal.h>
#include <termio.h>
#include <errno.h>

/*
 * This is a somewhat skeletal implementation of signal, it is
 * only intended to make the porting of benchmarks to the
 * LR33000 a little more straightforward.
 */

typedef void    Func ();
Func           *_sigintfunc;
jmp_buf         _sigintbuf;
jmp_buf         _savedintr;

#ifdef TEST
main ()
{
    char            buf[100];
    int             gotsig ();

    if (signal (SIGINT, gotsig) == -1) {
	printf ("signal: Bad return code\n");
	exit (-1);
    }
    setjmp (sjbuf);

    printf ("Waiting for signal.\n");
    for (;;) {
	printf ("> ");
	gets (buf);
	if (buf[0] == '.')
	    break;
	printf ("%s\n", buf);
    }
}

gotsig ()
{

    printf ("Got a signal\n");
    if (signal (SIGINT, gotsig) == -1) {
	printf ("signal: Bad return code\n");
	exit (-1);
    }
    longjmp (sjbuf, 0);
}
#endif

sig_t
signal (op, func)
     int             op;
     Func           *func;
{

    if (op == SIGINT) {
	if (func == SIG_IGN || func == SIG_DFL || func == SIG_HOLD) {
	    errno = EINVAL;
	    return (SIG_ERR);
	} else {
	    ioctl (STDIN, GETINTR, _savedintr);
	    _sigintfunc = func;
	    if (setjmp (_sigintbuf)) {	/* when INTR occurs */
		ioctl (STDIN, SETINTR, _savedintr);
		(*_sigintfunc) ();
	    } else {
		ioctl (STDIN, SETINTR, _sigintbuf);
		return (SIG_DFL);
	    }
	}
    }
    errno = EINVAL;
    return (SIG_ERR);
}
