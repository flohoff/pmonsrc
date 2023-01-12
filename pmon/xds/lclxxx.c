/* $Id: lclxxx.c,v 1.2 1996/01/16 14:25:19 chris Exp $ */
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
    
sbdmessage ("init_net\n");
    sysloglevel = LOG_NOTICE;
    e = getenv ("loglevel");
    if (e) {
	char *ee;
	u_long n = strtoul (e, &ee, 0);
	if (*ee == '\0' && n <= LOG_DEBUG) {
	  sysloglevel = n;
	}
	else {
	    CODE *code;
	    for (code = prioritynames; code->c_name; code++) {
		sbdmessage (code->c_name);
	      if (strcmp (code->c_name, ee) == 0)
		break;
	    }
	    if (code->c_name)
	      sysloglevel = code->c_val;
	    else
	      log (LOG_ERR, "bad $loglevel variable\n");
	}
    }

    /*
     * Init system global parameters
     */
    sbdmessage ("paraminit\n");
    paraminit ();

    /*
     * Initialise machine dependent bits
     */
    if (hwok) {
	sbdmessage ("machinit\n");
	machinit();
    }

    /*
     * Initialise "virtual memory" maps
     */
    sbdmessage ("vminit\n");
    {
	extern int memorysize;
	printf ("before vminit , memorysize = 0x%d\n", memorysize);
    }
    vminit();

    {
	extern int memorysize;
	printf ("after vminit , memorysize = 0x%d\n", memorysize);
    }
    /*
     * Initialise memory allocator
     */
    sbdmessage ("kmemnit\n");
    kmeminit();

    /*
     * Initialize callouts
     */
    sbdmessage ("callouts\n");
    callout = malloc(sizeof(struct callout) * ncallout, M_TEMP, M_NOWAIT);
    printf ("callout at 0x%x\n", callout);
    callfree = callout;
    for (i = 1; i < ncallout; i++)
      callout[i-1].c_next = &callout[i];

    if (hwok) {
	sbdmessage ("startrtclock\n");
      startrtclock(hz);
    }

    /*
     * Initialise mbufs
     */
	sbdmessage ("mbinit\n");
    mclrefcnt = (char *)malloc(NMBCLUSTERS+CLBYTES/MCLBYTES, M_MBUF, M_NOWAIT);
    bzero(mclrefcnt, NMBCLUSTERS+CLBYTES/MCLBYTES);
    mbinit();

    /*
     * Initialise network devices and protocols
     */
    if (hwok) {
	s = splhigh();
	sbdmessage ("eninit\n");
	eninit();			/* ethernet interface */
	sbdmessage ("loattach\n");
	loattach();			/* loopback interface */
	sbdmessage ("ifinit\n");
	ifinit();
	sbdmessage ("domaininit\n");
	domaininit();
	splx(s);
    }

    /* 
     * Initialise process table, we become first "process" 
     */
	sbdmessage ("init_proc\n");
    init_proc ();

    /* enable realtime clock interrupts */
    if (hwok) {
	sbdmessage ("enablertclock\n");

      enablertclock();		
    }

    boottime = time;
    spl0();

    if (hwok) {
	/* configure the default ethernet interface */
	sbdmessage ("ifconfig\n");
	ifconfig ("en0");
	hwinitdone = 1;
    }
	sbdmessage ("init_net done\n");
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
