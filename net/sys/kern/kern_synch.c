/* $Id: kern_synch.c,v 1.2 1996/01/16 14:21:24 chris Exp $ */
#include "param.h"
#include "proc.h"
#include "map.h"
#include "kernel.h"
#include "systm.h"
#include "syslog.h"
#include "signal.h"
#include "signalvar.h"

tsleep (chan, pri, mesg, timo)
    caddr_t chan;
    int pri, timo;
    char *mesg;
{
    struct proc *p = curproc;
    int catch = pri & PCATCH;
    int sig = 0;
    int s = splhigh ();
    int endtsleep ();

    if (p->p_wchan || !chan)
      panic ("tsleep");
    p->p_wmesg = mesg;
    p->p_wchan = chan;
    p->p_pri = pri;
    if (timo)
      timeout(endtsleep, (caddr_t)p, timo);

    if (catch) {
	p->p_flag |= SSINTR;
	if (sig = CURSIG(p)) {
	    unsleep (p);
	    p->p_stat = SRUN;
	    goto resume;
	}
    }

    p->p_stat = SSLEEP;
    do { 
	idle ();
    } while (p->p_wchan);

 resume:
    (void) splx (s);
    p->p_flag &= ~SSINTR;
    if (p->p_flag & STIMO) {
	p->p_flag &= ~STIMO;
	if (catch == 0 || sig == 0)
	  return (EWOULDBLOCK);
    }
    if (timo)
      untimeout(endtsleep, (caddr_t)p);
    if (catch && (sig || CURSIG(p))) {
	if (p->p_sigacts->ps_sigintr & sigmask(sig))
	  return (EINTR);
	return (ERESTART);
    }
    return (0);
}


sleep (chan, pri)
    caddr_t chan;
    int pri;
{
    tsleep (chan, pri, 0, 0);
}


unsleep (p)
    struct proc *p;
{
    int s = splhigh ();
    if (p->p_wchan) {
	p->p_wchan = 0;
    }
    (void) splx (s);
}


setrun (p)
    struct proc *p;
{
    int s = splhigh();
    if (p->p_stat != SSLEEP)
      panic ("setrun");
    unsleep (p);
    p->p_stat = SRUN;
    splx (s);
}


endtsleep(p)
    struct proc *p;
{
    int s = splhigh();
    if (p->p_wchan) {
	setrun(p);
	p->p_flag |= STIMO;
    }
    splx(s);
}

wakeup (chan)
    caddr_t chan;
{
    int s = splhigh ();
    if (curproc->p_wchan == chan)
      unsleep (curproc);
    (void) splx (s);
}

idle ()
{
    int s = spl0 ();
    scandevs ();
    (void) splx (s);
}

#include "../net/netisr.h"
#define DOISR(isr, handler) \
 if (netisr & (1 << (isr))) { \
	netisr &= ~(1 << (isr)); \
	handler (); \
 }

softnet ()
{
    DOISR(NETISR_RAW, rawintr);
#ifdef INET
    DOISR(NETISR_IP, ipintr);
#endif
#ifdef NS
    DOISR(NETISR_NS, nsintr);
#endif
#ifdef ISO
    DOISR(NETISR_ISO, clnlintr);
#endif
#ifdef CCITT
    DOISR(NETISR_CCITT, hdintr);
#endif
    DOISR(NETISR_SCLK, softclock);
}
