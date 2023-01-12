/* $Id: sbd.c,v 1.2 1999/04/22 16:35:04 chris Exp $ */
/*
 *	
 *	Copyright (c) 1998 ALGORITHMICS LIMITED 
 *	ALL RIGHTS RESERVED 
 *	
 *	THIS SOFTWARE PRODUCT CONTAINS THE UNPUBLISHED SOURCE
 *	CODE OF ALGORITHMICS LIMITED
 *	
 *	The copyright notices above do not evidence any actual 
 *	or intended publication of such source code.
 *	
 */

/*
 * sbd.c: CCUBE SBD C code
 */

#include "mips.h"
#include "termio.h"
#include "pmon.h"
#include "sbd.h"

#define MEG	0x100000

extern int ns16550 ();

/*
 * this initialisation table is used during basic initialisation
 */

const ConfigEntry ConfigTable[] =
{
    {(Addr)PA_TO_KVA1(UART0_BASE), 0, ns16550, 256, B9600},
    {(Addr)PA_TO_KVA1(UART1_BASE), 0, ns16550, 256, B9600},
    {(Addr)PA_TO_KVA1(UART2_BASE), 0, ns16550, 256, B9600},
/*    {(Addr)PA_TO_KVA1(UART3_BASE), 0, ns16550, 256, B9600}, */
    {0}};


/* return the current CPU type */
getmachtype ()
{
    switch ((Prid >> 8) & 0xff) {
    case 0x02:
    case 0x03:
	/* have we got an FPU? */
	return (initial_sr & SR_CU1) ? 3081 : 3051;
    case 0x07:
	return 3041;
    default:
	return 0;
    }
}


/* return name of this board */
const char *
sbdgetname ()
{
    return "CCUBE";
}


/* return CPU clock frequency */
int
sbdcpufreq ()
{
    /* should be calculated dynamically using timer */
    return MHZ*1000000;
}


/* very early low-level machine initialisation */
void
sbdmachinit ()
{
#if 1
    memorysize = 2 * MEG;	/* minimum possible */
#else
    memorysize = sizemem (CLIENTPC, LOCAL_MEM_SIZE);
#endif
}


int
sbdberrenb (int on)
{
    return 0;
}

int
sbdberrcnt (void)
{
    return 0;
}


/* initialise any local devices (except uarts, which are handled via ConfigTable) */
void
sbddevinit ()
{
}


/* enable any local interrupts which we will handle */
unsigned int
sbdenable (machtype)
{
    unsigned int sr;
    volatile ccubebcsr *bcsr;

    sbd_envreport ();

    bcsr->panic = BCSR_PANIC_BUS | BCSR_PANIC_DBG;
    wbflush ();

#if 0
    sr = SR_IBIT7 | SR_IEC;		/* enable panics */
#else
    sr = SR_IEC;		/* enable panics */
#endif
    (void) bis_sr (sr);
    return sr;
}

sbdstarttimer ()
{
}


/* poll any local devices */
sbdpoll ()
{
}	


/* control the l.e.d. blank enable */
void
sbdblank (int blankon)
{
}



void
sbdmessage (int line, const char *msg)
{
}


/* Low-level function to display a message on a display (4 chars packed
 * into one word); may be called before memory and .data have been
 * initialised. 
 */
void
sbddisplay (unsigned long msg)
{
}



sbdled (int on)
{
}



/*
 * dummy nvram handler for systems without nvram.
 */

char *
sbd_getenv (name)
    char *name;
{
    return 0;
}


int
sbd_setenv (char *name, char *value)
{
    return 1;
}


int
sbd_unsetenv (char *name)
{
    return 1;
}


void
sbd_mapenv (int (*func)(char *, char *))
{
}

int
sbd_envreport (void)
{
}


/* return a string describing the current debug/panic interrupt, if any */
char *
sbddbgintr (unsigned int cr)
{
    volatile ccubebcsr *bcsr = PA_TO_KVA1(BCSR_BASE);

    if (cr & CAUSE_IP7) {
	unsigned int irr = bcsr->panic;
	if (irr & BCSR_PANIC_DBG) {
	    /* software debounce debug button */
	    do {
		bcsr->panic = BCSR_PANIC_DBG;
		sbddelay (1000);
	    } while (bcsr->panic & BCSR_PANIC_DBG);
	    return "Debug";
	}
	if (irr & BCSR_PANIC_BUS) {
	    bcsr->panic = BCSR_PANIC_BUS;
	    return "Bus Timeout";
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
    return exc;
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

netopen ()	{return -1;}
netread ()	{return -1;}
netwrite ()	{return -1;}
netclose ()	{return -1;}
netlseek ()	{return -1;}
netioctl ()	{return -1;}
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

int
sbdpipefreq ()
{
    return MHZ * 1000000;
}


#ifndef RTC
long
sbd_gettime (void)
{
    return 0;
}
#endif

/* print board specific information */
void sbdmachinfo (void)
{
}
