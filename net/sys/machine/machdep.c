/* $Id: machdep.c,v 1.2 1996/01/16 14:21:39 chris Exp $ */
#include "sys/types.h"
#include "sys/param.h"
#include "kernel.h"
#include "systm.h"
#include "ioctl.h"
#include "socket.h"
#include "socketvar.h"
#include "syslog.h"
#include "time.h"

#include "net/if.h"
#include "net/if_types.h"
#include "net/netisr.h"

setsoftnet () 
{
    /* nothing to do, checked by spl0() */
}

setsoftclock ()
{
    /* simulated soft clock */
    schednetisr (NETISR_SCLK);
}

static int spl;


splx(newspl) 	
{
    int oldspl = spl;

    if (newspl < oldspl) {
	if (newspl < 7) {
	    spl = 7;
	    clkpoll ();
	    checkstack ();
	}
	if (newspl < 3) {
	    spl = 3;
	    ifpoll ();
	}
	if (newspl < 1 && netisr != 0) {
	    spl = 1;
	    softnet ();
	}
    }
    spl = newspl;
    return oldspl;
}

splhigh()	{return splx(7);}
splclock()	{return splx(7);}
spltty()	{return splx(5);}
splbio()	{return splx(4);}
splimp()	{return splx(3);}
splnet()	{return splx(1);}
splsoftclock()	{return splx(1);}
spl0()		{return splx(0);}


machinit ()
{
    /* no device interrupts please */
    splhigh ();
    sbdnetinit ();
}


machreset ()
{
    sbdnetreset ();
}


netpoll ()
{
    int s = splhigh ();
    splx (s);
}


ifpoll ()
{
    register struct ifnet *ifp;

    /* poll network interfaces */
    for (ifp = ifnet; ifp; ifp = ifp->if_next)
      if (ifp->if_ioctl)
	(*ifp->if_ioctl)(ifp, SIOCPOLL, 0);
}
