/* $Id: kern_syscalls.c,v 1.2 1996/01/16 14:21:25 chris Exp $ */
/*
 * kern_syscalls.c: fake system call glue for PROM net kernel
 */

#include "param.h"
#include "proc.h"
#include "kernel.h"
#include "systm.h"
#include "signal.h"
#include "signalvar.h"
#include "errno.h"
#include "syslog.h"

#include "machine/stdarg.h"

int errno;			/* has to be declared somewhere */

#define MAXARGS	6

static int
gensyscall (int ((*func)()), int nargs, int a1, va_list ap)
{
    extern int errno;
    struct args {long a[MAXARGS];} ua;
    struct proc *p = curproc;
    int rval[2], error, sig, i;

    if (p->p_stat != SNOTKERN)
      panic ("nested syscall");

    if (nargs > 0)
      ua.a[0] = a1;
    for (i = 1; i < nargs; i++)
      ua.a[i] = va_arg (ap, long);

    while (1) {
	p->p_stat = SRUN;
	rval[0] = 0;
	error = (*func) (p, &ua, rval);
	while (sig = CURSIG (p))		/* handle signals here */
	  psig (sig);
	p->p_stat = SNOTKERN;
	if (error != ERESTART) {
	    if (error) {
		errno = error;
		return -1;
	    }
	    return rval[0];
	}
    }
}

#define syscall(pub, pri, nargs) \
pub (int a1, ...) \
{ \
    extern int SYSCALL(pri) (); \
    va_list ap; \
    va_start (ap, a1); \
    gensyscall (SYSCALL(pri), nargs, a1, ap); \
    va_end (ap); \
}

syscall(soc_read, read, 3)
syscall(soc_write, write, 3)
syscall(soc_close, close, 1)
syscall(recvmsg, recvmsg, 3)
syscall(sendmsg, sendmsg, 3)
syscall(recvfrom, recvfrom, 6)
syscall(accept, accept, 3)
syscall(getpeername, getpeername, 3)
syscall(getsockname, getsockname, 3)
syscall(soc_dup, dup,  2)
syscall(soc_ioctl, ioctl, 3)
syscall(getdtablesize, getdtablesize, 0)
syscall(soc_dup2, dup2, 2)
syscall(soc_fcntl, fcntl, 3)
syscall(select, select, 5)
syscall(socket, socket, 3)
syscall(connect, connect, 3)
syscall(bind, bind,  3)
syscall(setsockopt, setsockopt,  5)
syscall(listen, listen,  2)
syscall(getsockopt, getsockopt,  5)
syscall(readv, readv,  3)
syscall(writev, writev,  3)
syscall(sendto, sendto,  6)
syscall(shutdown, shutdown,  2)
syscall(sigaction, sigaction, 3)
syscall(kernsigprocmask, sigprocmask, 2)
syscall(sigpending, sigpending, 0)
syscall(sigsuspend, sigsuspend, 1)
syscall(gettimeofday, gettimeofday, 2)
syscall(getitimer, getitimer, 2)
syscall(setitimer, setitimer, 3)

/*
 * TBD: user callable socket system call
 */
soc_syscall ()
{
    extern int errno;
    errno = EINVAL;
    return -1;
}


/* 
 * user callable exit (distinct from prom's exit routine) 
 */
soc_exit (rv)
{
    exit1 (curproc, rv & 0xff);
}

/*
 * Dummy system calls
 */

int getuid()	{return 0;}
int geteuid()	{return 0;}
int getegid()	{return 0;}
int getgid()	{return 0;}

int getpid()	
{
    return curproc->p_pid;
}

int getpgrp(pid)
{
    /* for us pgid == pid */
    return curproc->p_pid;
}

int gethostid()
{
    extern char *getenv();
    char *netaddr;
    int id;

    netaddr = getenv ("netaddr");
    if (netaddr && (id = inet_addr (netaddr)) != -1)
      return id;
    return 0;
}


gethostname (char *buf, int n)
{
    extern char *getenv();
    char *hostname;
    extern int errno;

    hostname = getenv ("hostname");
    if (!hostname)
      hostname = "pmon";

    if (n < strlen (hostname) + 1) {
	errno = EINVAL;
	return -1;
    }
    strcpy (buf, hostname);
    return 0;
}

/*
 * User-level signal handling (4.3bsd emulation on top of POSIX)
 */
sigvec(signo, sv, osv)
	int signo;
	struct sigvec *sv, *osv;
{
	int ret;

	if (sv)
		sv->sv_flags ^= SV_INTERRUPT;	/* !SA_INTERRUPT */
	ret = sigaction(signo, (struct sigaction *)sv, (struct sigaction *)osv);
	if (ret == 0 && osv)
		osv->sv_flags ^= SV_INTERRUPT;	/* !SA_INTERRUPT */
	return (ret);
}

/* 
 * The real sigprocmask system call takes only two args, and returns
 * the old mask.  This cover function munges the arguments appropriately.
 */
sigprocmask (int how, const sigset_t *set, sigset_t *oset)
{
    sigset_t new, old;
    int oerrno;

    if (!set) {
	how = SIG_BLOCK;
	new = 0;
    } else {
	new = *set;
    }

    oerrno = errno; errno = 0;
    old = kernsigprocmask (how, new);
    if (old == (sigset_t)-1 && errno)
      return -1;
    errno = oerrno;

    if (oset)
      *oset = old;
    return 0;
}

sigsetmask(mask)
	int mask;
{
	int omask, n;

	n = sigprocmask(SIG_SETMASK, (sigset_t *) &mask, (sigset_t *) &omask);
	if (n)
		return (n);
	return (omask);
}

sigblock(mask)
	int mask;
{
	int omask, n;

	n = sigprocmask(SIG_BLOCK, (sigset_t *) &mask, (sigset_t *) &omask);
	if (n)
		return (n);
	return (omask);
}

sigpause(mask)
	int mask;
{
	return (sigsuspend((int)&mask));
}

sigset_t _sigintr;		/* shared with siginterrupt */

sig_t
signal(s, a)
	int s;
	sig_t a;
{
	struct sigaction sa, osa;

	sa.sa_handler = a;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (!sigismember(&_sigintr, s))
		sa.sa_flags |= SA_RESTART;
	if (sigaction(s, &sa, &osa) < 0)
		return (BADSIG);
	return (osa.sa_handler);
}

/*
 * Set signal state to prevent restart of system calls
 * after an instance of the indicated signal.
 */
siginterrupt(sig, flag)
	int sig, flag;
{
	extern sigset_t _sigintr;
	struct sigaction sa;
	int ret;

	if ((ret = sigaction(sig, (struct sigaction *)0, &sa)) < 0)
		return (ret);
	if (flag) {
		sigaddset(&_sigintr, sig);
		sa.sa_flags &= ~SA_RESTART;
	} else {
		sigdelset(&_sigintr, sig);
		sa.sa_flags |= SA_RESTART;
	}
	return (sigaction(sig, &sa, (struct sigaction *)0));
}

