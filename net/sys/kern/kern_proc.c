/* $Id: kern_proc.c,v 1.2 1996/01/16 14:21:23 chris Exp $ */
/*
 * kern_proc.c: fake process creation and exit for IDT/sim
 * Copyright (c) 1993	Algorithmics Ltd.
 *
 * XXX what about stuff malloc'ed by sub-process XXX
 */

#include "param.h"
#include "resource.h"
#include "resourcevar.h"
#include "proc.h"
#include "kernel.h"
#include "systm.h"
#include "filedesc.h"
#include "signal.h"
#include "signalvar.h"
#include "errno.h"
#include "syslog.h"

#ifndef NPROC
#define NPROC 3
#endif

static struct sigacts sa[NPROC];
static struct pstats stats[NPROC];
static struct filedesc0 fd0;

struct proc *curproc;
struct proc proc[NPROC];
int maxproc;
int nprocs;
int nextpid;

/*
 * Spawn a new "process", which runs til completion.
 * Callable directly by user.
 */
int
spawn (name, func, argc, argv)
    char *name;
    int (*func)();
    int argc;
    char **argv;
{
    struct proc *p = curproc;
    struct proc *np;
    extern int errno, nextpid;
    int sig, rv;

    if (nprocs >= maxproc) {
	tablefull ("proc");
	errno = EAGAIN;
	return (-1);
    }

    np = &proc[nprocs++];
    if (np->p_stat != SZOMB)
      panic ("spawn");
    
    np->p_pid = ++nextpid;
    np->p_comm = name;

    /* clear fields */
    np->p_wchan = 0;
    np->p_wmesg = 0;
    np->p_sig = 0;

    /* copy signal state, and update */
    np->p_sigmask = p->p_sigmask;
    np->p_sigignore = p->p_sigignore;
    np->p_sigcatch = p->p_sigcatch;
    *np->p_sigacts = *p->p_sigacts;
    execsigs (np);

    /* clear real-time timer */
    timerclear (&np->p_realtimer.it_interval);
    timerclear (&np->p_realtimer.it_value);

    /* copy virtual interval timers */
    bcopy (p->p_stats, np->p_stats, sizeof(struct pstats));

    /* copy file descriptors */
    np->p_fd = fdcopy (p);

    if (rv = setjmp (np->p_exitbuf)) {
	/* new process has exited */
	curproc = p;
	/* handle newly pending signals */
	while (sig = CURSIG (p))
	  psig (sig);
	p->p_stat = SNOTKERN;
	spl0 ();
	return rv & 0xffff;
    }

    /* new process is active */
    p->p_stat = SRUN;		/* pretend old proc is locked in kernel */
    np->p_stat = SNOTKERN;	/* new proc is not in kernel */
    curproc = np;		/* new proc becomes current proc */

    /* make sure resolver state is initialised */
    _res_reset ();

    exit1 (np, (*func)(argc, argv) & 0xff);
}


struct proc *
pfind (pid)
{
    struct proc *p;
    for (p = proc; p < &proc[maxproc]; p++)
      if (p->p_stat != SZOMB && p->p_pid == pid)
	return (p);
    return (0);
}


/*
 * Internal "process" exit handler
 */
exit1 (p, rv)
    struct proc *p;
{
    int s;

    if (p->p_stat == SZOMB)
      panic ("zombie exit");

    s = splhigh ();
    p->p_sigignore = ~0;
    p->p_sig = 0;
    untimeout(realitexpire, (caddr_t)p);

    fdfree (p);
    --nprocs;
    (void) splx (s);

    p->p_stat = SZOMB;
    if (p == curproc)
      curproc = 0;

    if ((rv >> 8) != SIGKILL) {	/* XXX see procreset() below */
	if (p == &proc[0])
	  panic ("prom process died");
	else
	  longjmp (p->p_exitbuf, rv | 0x10000 /* force non-zero */);
	/* neither return */
    }
}


sigexit (p, sig)
    struct proc *p;
    int sig;
{
    if (sig != SIGINT)
      log (LOG_ERR, "process %d (%s) dying from signal %d\n",
	   p->p_pid, p->p_comm ? p->p_comm : "noname", sig & 0xff);
    exit1 (p, sig << 8);
}


extern int	stack[];

static enum {GREEN, YELLOW, RED, OFF} stackalert = OFF;
#define YELLOW_LIMIT		(2048/sizeof(int))
#define RED_LIMIT		(512/sizeof(int))
#define STACK_MAGIC		0xBADF00D

init_proc ()
{
    struct proc *p;
    int i;

    maxproc = NPROC;
    nprocs = 0;
    nextpid = 0;
    curproc = 0;

    bzero (&fd0, sizeof(fd0));
    fd0.fd_fd.fd_ofiles = fd0.fd_dfiles;
    fd0.fd_fd.fd_ofileflags = fd0.fd_dfileflags;
    fd0.fd_fd.fd_nfiles = NDFILE;
    fd0.fd_fd.fd_refcnt = 100;	/* never free */

    bzero (proc, sizeof (proc));
    bzero (sa, sizeof (sa));
    bzero (stats, sizeof (stats));
    for (p = proc, i = 0; p < &proc[NPROC]; p++, i++) {
      p->p_sigacts = &sa[i];
      p->p_stats = &stats[i];
    }

    p = &proc[nprocs++];
    p->p_stat = SNOTKERN;
    p->p_fd = &fd0.fd_fd;
    p->p_comm = "pmon";
    p->p_pid = ++nextpid;
    
    /* initial signal state */
    siginit (p);

    curproc = p;

    stack[YELLOW_LIMIT] = STACK_MAGIC;
    stack[RED_LIMIT] = STACK_MAGIC;
    stackalert = GREEN;
}


checkstack ()
{
    if (stackalert < RED && stack[RED_LIMIT] != STACK_MAGIC) {
	stackalert = RED;
	log (LOG_CRIT, "process \"%s\": stack overflow\n",
	     curproc ? curproc->p_comm : "???");
	if (curproc)
	  psignal (curproc, SIGSEGV);
	else
	  panic ("stack overflow");
    } else if (stackalert < YELLOW && stack[YELLOW_LIMIT] != STACK_MAGIC) {
	stackalert = YELLOW;
	log (LOG_WARNING, "process \"%s\": stack yellow alert\n",
	     curproc ? curproc->p_comm : "???");
    }
}


/*
 * Clean up on promexit()
 */
procreset ()
{
    struct proc *p;

    /* try to clean up processes and hanging connections */
    for (p = &proc[NPROC-1]; p >= proc; p--)
      if (p->p_stat != SZOMB)
	/* XXX see exit1() for special handling of SIGKILL */
	psignal (curproc = p, SIGKILL);

    if (nprocs > 0)
      panic ("procreset: couldn't kill all processes");

    init_proc ();
}
