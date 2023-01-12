/* $Id: kern_sig.c,v 1.4 1998/09/21 14:23:24 chris Exp $ */
#include "param.h"
#include "proc.h"
#include "map.h"
#include "kernel.h"
#include "systm.h"
#include "syslog.h"

#define SIGPROP
#include "signal.h"
#include "signalvar.h"

/* ARGSUSED */
SYSCALL(sigaction)(p, v, retval)
	struct proc *p;
	void *v;
	int *retval;
{
	register struct args {
		int	signo;
		struct	sigaction *nsa;
		struct	sigaction *osa;
	} *uap = v;
	struct sigaction vec;
	register struct sigaction *sa;
	register struct sigacts *ps = p->p_sigacts;
	register int sig;
	int bit, error;

	sig = uap->signo;
	if (sig <= 0 || sig >= NSIG || sig == SIGKILL || sig == SIGSTOP)
		return (EINVAL);
	sa = &vec;
	if (uap->osa) {
		sa->sa_handler = ps->ps_sigact[sig];
		sa->sa_mask = ps->ps_catchmask[sig];
		bit = sigmask(sig);
		sa->sa_flags = 0;
#ifndef PROM
		if ((ps->ps_sigonstack & bit) != 0)
			sa->sa_flags |= SA_ONSTACK;
		if (p->p_flag & SNOCLDSTOP)
			sa->sa_flags |= SA_NOCLDSTOP;
#endif
		if ((ps->ps_sigintr & bit) == 0)
			sa->sa_flags |= SA_RESTART;
		if (error = copyout((caddr_t)sa, (caddr_t)uap->osa,
		    sizeof (vec)))
			return (error);
	}
	if (uap->nsa) {
		if (error = copyin((caddr_t)uap->nsa, (caddr_t)sa,
		    sizeof (vec)))
			return (error);
		setsigvec(p, sig, sa);
	}
	return (0);
}

setsigvec(p, sig, sa)
	register struct proc *p;
	int sig;
	register struct sigaction *sa;
{
	register struct sigacts *ps = p->p_sigacts;
	register int bit;

	bit = sigmask(sig);
	/*
	 * Change setting atomically.
	 */
	(void) splhigh();
	ps->ps_sigact[sig] = sa->sa_handler;
	ps->ps_catchmask[sig] = sa->sa_mask &~ sigcantmask;
	if ((sa->sa_flags & SA_RESTART) == 0)
		ps->ps_sigintr |= bit;
	else
		ps->ps_sigintr &= ~bit;
#ifndef PROM
	if (sa->sa_flags & SA_ONSTACK)
		ps->ps_sigonstack |= bit;
	else
		ps->ps_sigonstack &= ~bit;
	if (sig == SIGCHLD) {
		if (sa->sa_flags & SA_NOCLDSTOP)
			p->p_flag |= SNOCLDSTOP;
		else
			p->p_flag &= ~SNOCLDSTOP;
	}
#endif
	/*
	 * Set bit in p_sigignore for signals that are set to SIG_IGN,
	 * and for signals set to SIG_DFL where the default is to ignore.
	 * However, don't put SIGCONT in p_sigignore,
	 * as we have to restart the process.
	 */
	if (sa->sa_handler == SIG_IGN ||
	    (sigprop[sig] & SA_IGNORE && sa->sa_handler == SIG_DFL)) {
		p->p_sig &= ~bit;		/* never to be seen again */
		if (sig != SIGCONT)
			p->p_sigignore |= bit;	/* easier in psignal */
		p->p_sigcatch &= ~bit;
	} else {
		p->p_sigignore &= ~bit;
		if (sa->sa_handler == SIG_DFL)
			p->p_sigcatch &= ~bit;
		else
			p->p_sigcatch |= bit;
	}
	(void) spl0();
}

/*
 * Initialize signal state for process 0;
 * set to ignore signals that are ignored by default.
 */
void
siginit(p)
	struct proc *p;
{
	register int i;

	for (i = 0; i < NSIG; i++)
		if (sigprop[i] & SA_IGNORE && i != SIGCONT)
			p->p_sigignore |= sigmask(i);
}

/*
 * Reset signals for an exec of the specified process.
 */
void
execsigs(p)
	register struct proc *p;
{
	register struct sigacts *ps = p->p_sigacts;
	register int nc, mask;

	/*
	 * Reset caught signals.  Held signals remain held
	 * through p_sigmask (unless they were caught,
	 * and are now ignored by default).
	 */
	while (p->p_sigcatch) {
		nc = ffs((long)p->p_sigcatch);
		mask = sigmask(nc);
		p->p_sigcatch &= ~mask;
		if (sigprop[nc] & SA_IGNORE) {
			if (nc != SIGCONT)
				p->p_sigignore |= mask;
			p->p_sig &= ~mask;
		}
		ps->ps_sigact[nc] = SIG_DFL;
	}
#ifndef PROM
	/*
	 * Reset stack state to the user stack.
	 * Clear set of signals caught on the signal stack.
	 */
	ps->ps_onstack = 0;
	ps->ps_sigsp = 0;
	ps->ps_sigonstack = 0;
#endif
}

/*
 * Manipulate signal mask.
 * Note that we receive new mask, not pointer,
 * and return old mask as return value;
 * the library stub does the rest.
 */
SYSCALL(sigprocmask)(p, v, retval)
	register struct proc *p;
	void *v;
	int *retval;
{
        register struct args {
		int	how;
		sigset_t mask;
	} *uap = v;

	int error = 0;

	*retval = p->p_sigmask;
	(void) splhigh();

	switch (uap->how) {
	case SIG_BLOCK:
		p->p_sigmask |= uap->mask &~ sigcantmask;
		break;

	case SIG_UNBLOCK:
		p->p_sigmask &= ~uap->mask;
		break;

	case SIG_SETMASK:
		p->p_sigmask = uap->mask &~ sigcantmask;
		break;
	
	default:
		error = EINVAL;
		break;
	}
	(void) spl0();
	return (error);
}

/* ARGSUSED */
SYSCALL(sigpending)(p, uap, retval)
	struct proc *p;
	void *uap;
	int *retval;
{

	*retval = p->p_sig;
	return (0);
}

#ifdef COMPAT_43
/*
 * Generalized interface signal handler, 4.3-compatible.
 */
/* ARGSUSED */
SYSCALL(osigvec)(p, uap, retval)
	struct proc *p;
	register struct args {
		int	signo;
		struct	sigvec *nsv;
		struct	sigvec *osv;
	} *uap;
	int *retval;
{
	struct sigvec vec;
	register struct sigacts *ps = p->p_sigacts;
	register struct sigvec *sv;
	register int sig;
	int bit, error;

	sig = uap->signo;
	if (sig <= 0 || sig >= NSIG || sig == SIGKILL || sig == SIGSTOP)
		return (EINVAL);
	sv = &vec;
	if (uap->osv) {
		*(sig_t *)&sv->sv_handler = ps->ps_sigact[sig];
		sv->sv_mask = ps->ps_catchmask[sig];
		bit = sigmask(sig);
		sv->sv_flags = 0;
		if ((ps->ps_sigonstack & bit) != 0)
			sv->sv_flags |= SV_ONSTACK;
		if ((ps->ps_sigintr & bit) != 0)
			sv->sv_flags |= SV_INTERRUPT;
		if (p->p_flag & SNOCLDSTOP)
			sv->sv_flags |= SA_NOCLDSTOP;
		if (error = copyout((caddr_t)sv, (caddr_t)uap->osv,
		    sizeof (vec)))
			return (error);
	}
	if (uap->nsv) {
		if (error = copyin((caddr_t)uap->nsv, (caddr_t)sv,
		    sizeof (vec)))
			return (error);
		sv->sv_flags ^= SA_RESTART;	/* opposite of SV_INTERRUPT */
		setsigvec(p, sig, (struct sigaction *)sv);
	}
	return (0);
}

SYSCALL(osigblock)(p, uap, retval)
	register struct proc *p;
	struct args {
		int	mask;
	} *uap;
	int *retval;
{

	(void) splhigh();
	*retval = p->p_sigmask;
	p->p_sigmask |= uap->mask &~ sigcantmask;
	(void) spl0();
	return (0);
}

SYSCALL(osigsetmask)(p, uap, retval)
	struct proc *p;
	struct args {
		int	mask;
	} *uap;
	int *retval;
{

	(void) splhigh();
	*retval = p->p_sigmask;
	p->p_sigmask = uap->mask &~ sigcantmask;
	(void) spl0();
	return (0);
}
#endif

/*
 * Suspend process until signal, providing mask to be set
 * in the meantime.  Note nonstandard calling convention:
 * libc stub passes mask, not pointer, to save a copyin.
 */
/* ARGSUSED */
SYSCALL(sigsuspend)(p, v, retval)
	register struct proc *p;
	void *v;
	int *retval;
{
        register struct args {
		sigset_t mask;
	} *uap = v;
	register struct sigacts *ps = p->p_sigacts;

	/*
	 * When returning from sigpause, we want
	 * the old mask to be restored after the
	 * signal handler has finished.  Thus, we
	 * save it here and mark the proc structure
	 * to indicate this (should be in sigacts).
	 */
	ps->ps_oldmask = p->p_sigmask;
	ps->ps_flags |= SA_OLDMASK;
	p->p_sigmask = uap->mask &~ sigcantmask;
	(void) tsleep((caddr_t) ps, PPAUSE|PCATCH, "pause", 0);
	/* always return EINTR rather than ERESTART... */
	return (EINTR);
}

#ifndef PROM
/* ARGSUSED */
sigstack(p, uap, retval)
	struct proc *p;
	register struct args {
		struct	sigstack *nss;
		struct	sigstack *oss;
	} *uap;
	int *retval;
{
	struct sigstack ss;
	int error = 0;

	if (uap->oss && (error = copyout((caddr_t)&p->p_sigacts->ps_sigstack,
	    (caddr_t)uap->oss, sizeof (struct sigstack))))
		return (error);
	if (uap->nss && (error = copyin((caddr_t)uap->nss, (caddr_t)&ss,
	    sizeof (ss))) == 0)
		p->p_sigacts->ps_sigstack = ss;
	return (error);
}

/* ARGSUSED */
kill(cp, uap, retval)
	register struct proc *cp;
	register struct args {
		int	pid;
		int	signo;
	} *uap;
	int *retval;
{
	register struct proc *p;
	register struct pcred *pc = cp->p_cred;

	if ((unsigned) uap->signo >= NSIG)
		return (EINVAL);
	if (uap->pid > 0) {
		/* kill single process */
		p = pfind(uap->pid);
		if (p == 0)
			return (ESRCH);
		if (!CANSIGNAL(cp, pc, p, uap->signo))
			return (EPERM);
		if (uap->signo)
			psignal(p, uap->signo);
		return (0);
	}
	switch (uap->pid) {
	case -1:		/* broadcast signal */
		return (killpg1(cp, uap->signo, 0, 1));
	case 0:			/* signal own process group */
		return (killpg1(cp, uap->signo, 0, 0));
	default:		/* negative explicit process group */
		return (killpg1(cp, uap->signo, -uap->pid, 0));
	}
	/* NOTREACHED */
}

#ifdef COMPAT_43
/* ARGSUSED */
okillpg(p, uap, retval)
	struct proc *p;
	register struct args {
		int	pgid;
		int	signo;
	} *uap;
	int *retval;
{

	if ((unsigned) uap->signo >= NSIG)
		return (EINVAL);
	return (killpg1(p, uap->signo, uap->pgid, 0));
}
#endif

/*
 * Common code for kill process group/broadcast kill.
 * cp is calling process.
 */
killpg1(cp, signo, pgid, all)
	register struct proc *cp;
	int signo, pgid, all;
{
	register struct proc *p;
	register struct pcred *pc = cp->p_cred;
	struct pgrp *pgrp;
	int nfound = 0;
	
	if (all)	
		/* 
		 * broadcast 
		 */
		for (p = allproc; p != NULL; p = p->p_nxt) {
			if (p->p_pid <= 1 || p->p_flag&SSYS || 
			    p == cp || !CANSIGNAL(cp, pc, p, signo))
				continue;
			nfound++;
			if (signo)
				psignal(p, signo);
		}
	else {
		if (pgid == 0)		
			/* 
			 * zero pgid means send to my process group.
			 */
			pgrp = cp->p_pgrp;
		else {
			pgrp = pgfind(pgid);
			if (pgrp == NULL)
				return (ESRCH);
		}
		for (p = pgrp->pg_mem; p != NULL; p = p->p_pgrpnxt) {
			if (p->p_pid <= 1 || p->p_flag&SSYS ||
			    p->p_stat == SZOMB || !CANSIGNAL(cp, pc, p, signo))
				continue;
			nfound++;
			if (signo)
				psignal(p, signo);
		}
	}
	return (nfound ? 0 : ESRCH);
}
#endif /* !PROM */

void gsignal (pgid, sig)
{
    /* each process is its own group */
    if (curproc)
	psignal (curproc, sig);
}


void psignal (p, sig)
    struct proc *p;
    int sig;
{
    extern int _insyscall;
    int mask, prop;
    sig_t action;
    int s;

    if ((unsigned)sig >= NSIG || sig == 0)
      panic("psignal sig");
    mask = sigmask(sig);
    prop = sigprop[sig];

    if (p->p_sigignore & mask)
      return;
    if (p->p_sigmask & mask)
      action = SIG_HOLD;
    else if (p->p_sigcatch & mask)
      action = SIG_CATCH;
    else
      action = SIG_DFL;

    p->p_sig |= mask;
    if (action == SIG_HOLD)
      return;

    s = splhigh ();
    switch (p->p_stat) {
    case SSLEEP:
	if (p->p_flag & SSINTR)
	  /* sleeping interruptibly, wake up */
	  setrun (p);
	break;
    case SNOTKERN:
	/* exception? while not in syscall */
#ifdef PROM
	if (p == curproc)
#else
	if (p == curproc && prop & SA_CORE)
#endif
	  psig (sig);
	break;
    default:
	/* inside net kernel, will be picked up on soon */
	break;
    }
#ifdef PROM
    /* procreset needs handling NOW */
    if (p == curproc && sig == SIGKILL)
      psig (sig);
#endif
    (void) splx (s);
}


issig (p)
    struct proc *p;
{
    register int sig, mask, prop;

    for (;;) {
	mask = p->p_sig &~ p->p_sigmask;
	if (mask == 0)
	  return (0);
	sig = ffs((long)mask);
	mask = sigmask(sig);
	prop = sigprop[sig];
	if (mask & p->p_sigignore) {
	    p->p_sig &= ~mask;
	    continue;
	}
	if (p->p_sigacts->ps_sigact[sig] != SIG_IGN) {
	    if (!(prop & SA_IGNORE))
	      return (sig);
	}
	p->p_sig &= ~mask;
    }
}

void psig (sig)
{
    struct proc *p = curproc;
    struct sigacts *ps = p->p_sigacts;
    sig_t action;
    int mask, returnmask;
    int ostat;

    mask = sigmask (sig);
    p->p_sig &= ~mask;
    action = ps->ps_sigact[sig];
    if (action == SIG_DFL) {
	sigexit (p, sig);
    } else {
	int s = splhigh ();
	checkstack ();
	if (ps->ps_flags & SA_OLDMASK) {
	    returnmask = ps->ps_oldmask;
	    ps->ps_flags &= ~SA_OLDMASK;
	} else
	  returnmask = p->p_sigmask;
	p->p_sigmask |= ps->ps_catchmask[sig] | mask;
	(void) spl0 ();
	ostat = p->p_stat;
	p->p_stat = SNOTKERN;
	(*action) (sig);
	p->p_stat = ostat;
	(void) splhigh ();
	p->p_sigmask = returnmask;
	checkstack ();
	(void) splx (s);
    }
}
