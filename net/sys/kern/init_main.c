/* $Id: init_main.c,v 1.2 1996/01/16 14:21:18 chris Exp $ */
#include "param.h"
#include "kernel.h"
#include "map.h"
#include "proc.h"
#include "systm.h"
#include "callout.h"
#include "malloc.h"
#include "mbuf.h"
#include "protosw.h"
#define SYSLOG_NAMES
#include "syslog.h"

#include "vm/vm.h"

static int hwinitdone;
int sysloglevel;

init_net (int hwok)
{
    extern char *getenv();
    char *e;
    int i, s;
    
    sysloglevel = LOG_NOTICE;
    if (e = getenv ("loglevel")) {
	char *ee;
	u_long n = strtoul (e, &ee, 0);
	if (*ee == '\0' && n <= LOG_DEBUG)
	  sysloglevel = n;
	else {
	    CODE *code;
	    for (code = prioritynames; code->c_name; code++)
	      if (strcmp (code->c_name, ee) == 0)
		break;
	    if (code->c_name)
	      sysloglevel = code->c_val;
	    else
	      log (LOG_ERR, "bad $loglevel variable\n");
	}
    }

    /*
     * Init system global parameters
     */
    paraminit ();

    /*
     * Initialise machine dependent bits
     */
    if (hwok)
      machinit();

    /*
     * Initialise "virtual memory" maps
     */
    vminit();

    /*
     * Initialise memory allocator
     */
    kmeminit();

    /*
     * Initialize callouts
     */
    callout = malloc(sizeof(struct callout) * ncallout, M_TEMP, M_NOWAIT);
    callfree = callout;
    for (i = 1; i < ncallout; i++)
      callout[i-1].c_next = &callout[i];

    if (hwok)
      startrtclock(hz);

    /*
     * Initialise mbufs
     */
    mclrefcnt = (char *)malloc(NMBCLUSTERS+CLBYTES/MCLBYTES, M_MBUF, M_NOWAIT);
    bzero(mclrefcnt, NMBCLUSTERS+CLBYTES/MCLBYTES);
    mbinit();

    /*
     * Initialise network devices and protocols
     */
    if (hwok) {
	s = splhigh();
	eninit();			/* ethernet interface */
	loattach();			/* loopback interface */
	ifinit();
	domaininit();
	splx(s);
    }

    /* 
     * Initialise process table, we become first "process" 
     */
    init_proc ();

    /* enable realtime clock interrupts */
    if (hwok)
      enablertclock();		

    boottime = time;
    spl0();

    if (hwok) {
	/* configure the default ethernet interface */
	ifconfig ("en0");
	hwinitdone = 1;
    }
}


reset_net ()
{
    if (callout) {
	(void) splhigh ();
	machreset ();		/* reset m/c specific code */
	procreset ();		/* zap any running processes */
	if (hwinitdone) {
	    ifubareset ();	/* reset ethernet controllers */
	    enablertclock ();	/* reenable clock i/us */
	}
	(void) spl0 ();
    }
}
