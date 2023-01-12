/* $Id: sbd.c,v 1.5 1999/04/22 16:35:07 chris Exp $ */
#include "mips.h"
#include "termio.h"
#include "pmon.h"
#include "cirrms/sbd.h"
#include "cirrms/m82510.h"

extern int m82510 ();

const ConfigEntry ConfigTable[] =
{
    {(Addr)SIO0BASE, 0, m82510, 256, B9600},
    {(Addr)SIO1BASE, 0, m82510, 256, B9600},
    {(Addr)SIO2BASE, 0, m82510, 256, B9600},
    {0}};


/* return the current CPU type */
getmachtype ()
{
    return (3081);
}


/* return name of this board */
const char *
sbdgetname ()
{
    return "CIRRMS";
}


/* return CPU clock frequency */
int
sbdcpufreq ()
{
    /* should be calculated dynamically using timer */
    return 25000000;
}


/* very early low-level machine initialisation */
void
sbdmachinit ()
{
    sbd_envinit ();
    (void) *(volatile unsigned int *)IACKPANIC;
    memorysize = sizemem (CLIENTPC, MEMSIZE);
}


/* initialise any local devices (except uarts which are handled via ConfigTable) */
void
sbddevinit ()
{
}


void
sbdpoll ()
{
    /* poll any special devices */
}


/* enable any local interrupts which we will handle */
unsigned int
sbdenable (machtype)
{
    /* enable Panic and FPU interrupts */
    unsigned int sr = SR_IBIT7 | SR_IBIT6 | SR_IEC;
    sbd_envreport ();		/* report environent errors */
    (void) bis_sr (sr);
    return sr;
}


/* enable/disable bus errors */
sbdberrenb (int enb)
{
    return 0;
}


/* return bus error count */
sbdberrcnt ()
{
    return 0;
}


/* control the l.e.d. blank enable */
void
sbdblank (int blankon)
{
}


/* display a message on the l.e.d. (four chars packed into one word) */
void
sbddisplay (unsigned long msg)
{
}


/* return a string describing the current debug/panic interrupt, if any */
char *
sbddbgintr (unsigned int cr)
{
    if (cr & CAUSE_IP7) {
	volatile byte *irr = (byte *)IRR;
	volatile byte *iack = (byte *)IACKPANIC;
	unsigned char ip;

	/* read and acknowledge current interrupt */
	ip = *irr; *iack = 0; wbflush();

	if (ip & IRR_WBUSERR)
	  return "Write bus error";
	if (ip & IRR_WATCHDOG)
	  return "Watchdog timeout";
	if (ip & IRR_DEBUG) {
	    /* Debounce the debug button (I hope that no other panics
	     * arrive in the meantime). */
	    do {
		int i;
		*iack = 0; wbflush();
		for (i = 20000; i > 0; i--) continue;
	    } while (*irr & IRR_DEBUG);
	    return "Debug";
	}
    }
    return 0;
}


/* called on a fatal prom exception: display it on l.e.d, (if present) */
char *
sbdexception (epc, cause, ra, extra, exc)
    unsigned long epc, cause, ra, extra;
    char *exc;
{
    return 0;
}


/*
 * floating point emulator stubs
 */

_fpstatesz ()
{
    return 0;
}

_fpstate ()
{
    return -1;
}

cop1 ()
{
    printf ("\ncop1 called\n");
    cliexit ();
}


/* gsignal() is called when a keyboard interrupt (^C) is detected */
gsignal (jb, sig)
    jmp_buf *jb;
    int sig;
{
    if (jb)
      longjmp (jb, 1);
}


/* stubs for unused network support */
tftpopen ()	{return -1;}
tftpread ()	{return -1;}
tftpwrite ()	{return -1;}
tftpclose ()	{return -1;}
tftplseek ()	{return -1;}
netpoll ()	{}


/*
 * flash support stub
 */

void
sbd_flashinfo (void **flashbuf, int *flashsize)
{
    *flashbuf = (void *)0;
    *flashsize = 0;
}


int
sbd_flashprogram (unsigned char *data, unsigned int size, unsigned int offset)
{
    printf ("This target does not support FLASH devices\n");
    return -1;
}

/* print board specific information */
void sbdmachinfo (void)
{
}
