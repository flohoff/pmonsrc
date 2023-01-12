/* $Id: sbd.c,v 1.4 2000/03/28 00:21:36 nigel Exp $ */
/* glacier/sbd.c */

#include <mips.h>
#include <pmon.h>
#include <termio.h>
#include <glacier/sbd.h>

#define MAXDISPLAYCOLS	16

extern int gser();
extern int gpar();

ConfigEntry     ConfigTable[] =
{
    {(Addr)0, 0, gser, 256, B9600},
    {(Addr)0, 0, gpar, 512, B9600},
    {0}};

/* NetBSD style autoconf data */
#include "pci/pcivar.h"
#include "pci/device.h"

extern struct cfdriver gsercd;
extern struct cfdriver gparcd;

#define NORM FSTATE_NOTFOUND
#define STAR FSTATE_STAR

struct cfdata cfdata[] = {
	/* driver     unit state    loc     flags parents ivstubs */
/*  0: gser0 */
	{&gsercd,	 	0, NORM,    0,      0, 	  0, 	  0},
/*  1: gpar0 */
	{&gparcd,	 	0, NORM,    0,      0, 	  0, 	  0},
	{0}
};



extern void display_init (void);
extern void display_size (int *, int *);
extern void display_message (int, const char *);

static int displaycols;
static int displayrows;


const char *
sbdgetname ()
{
    return "Glacier";
}



/* early low-level machine initialisation */
void
sbdmachinit ()
{
    /* initialise environment from eerom */
    sbd_envinit ();

    if (memorysize == 0)
	memorysize = sizemem (CLIENTPC, LOCAL_MEM_SIZE);
}


sbd_dispinit ()
{
    /* initialise display */
}


/* control the display blank enable */
void
sbdblank (int blankon)
{
}


void
sbddevinit ()
{
    /* program any local devices */
    /* PCI */
    int s = bic_sr (SR_IE);
    int i;
    for (i = 0; cfdata[i].cf_driver; i++)
	cfdata[i].cf_fstate = FSTATE_NOTFOUND;
    {
	extern int pciverbose;
	char *v = getenv("pciverbose");
	if (v)
	    pciverbose = atol (v);
    }
    pci_configure ();
    (void) set_sr (s);
}



void
sbdpoll ()
{
    /* poll any special devices */
}


unsigned int
sbdenable (int machtype)
{
    unsigned int sr;

    /* now is the chance to report any early eerom failures */
    sbd_envreport ();

    /* no interrupts enabled */
    sr = SR_IE;

    (void) bis_sr (sr);
    return sr;
}


char *
sbddbgintr (unsigned int cr)
{
    return 0;
}


/* display a long message on the display, scrolling if necessary. */
void
sbdmessage (int line, const char *msg)
{
}

/* display a message and then pause so that it can be read */
static void 
display (const char *msg)
{
}


/* display a register name and its value, then pause */
static void
xdisplay (char *s, unsigned int x)
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

char *
sbdexception (epc, cause, ra, extra, exc)
    unsigned long   epc, cause, ra, extra;
    char *exc;
{
    static char excbuf[50];
    char *s;
    int i;

    if (!exc) {
	if ((cause & CAUSE_EXCMASK) == CEXC_INT && (s = sbddbgintr (cause)))
	  sprintf (excbuf, "%s interrupt", s);
        else if (cause == CEXC_CACHE) 
	  sprintf (excbuf, "Uncorrectable Cache Error");
	else
	  sprintf (excbuf, "PMON %s exception", 
		   getexcname (cause & CAUSE_EXCMASK));
	exc = excbuf;
    }

    display  (exc);
    if (cause != CEXC_CACHE)
	xdisplay ("Cause", cause);
    xdisplay ("EPC", epc);
    xdisplay ("RA", ra);
    switch (cause & CAUSE_EXCMASK) {
    case CEXC_MOD:
    case CEXC_TLBL:
    case CEXC_TLBS:
    case CEXC_ADEL:
    case CEXC_ADES:
	xdisplay ("BadVaddr", extra);
	break;
    case CEXC_CACHE:
	xdisplay ("CacheErr", extra);
	break;
    }
    display ("");
    return exc;
}


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


getmachtype ()
{
    switch ((Prid >> 8) & 0xff) {
    case 0x0b:
	return 4300;
    case 0x22:
	return 4640;
    default:
	return 0;
    }
}

#ifndef INET
/* gsignal() is called when a keyboard interrupt (^C) is detected */
gsignal (jmp_buf *jb, int sig)
{
    if (jb) longjmp (jb, 1);
}

netopen ()	{return -1;}
netread ()	{return -1;}
netwrite ()	{return -1;}
netclose ()	{return -1;}
netlseek ()	{return -1;}
netioctl ()	{return -1;}
netpoll ()	{}

#ifdef RTC
time_t __time (void)
{
    return (sbd_gettime ());
}
#endif
#endif


static unsigned long pipefreq = 0;
int cpuspeed = 15;

/* hand tune delay */
static void 
probefreq(void)
{
}


unsigned long sbdpipefreq (void)
{
    return MHZ;
}

unsigned long sbdcpufreq (void)
{
    unsigned long freq;
    freq = sbdpipefreq ();
    /* R4032 specific */
    switch ((Config & CFG_ECMASK) >> CFG_ECSHIFT) {
    case 0: return freq / 2;
    case 1: return freq / 3;
    case 6: return freq;
    case 7: return (2 * freq) / 3;
    }
    return (freq);
}


#ifndef INET
void
panic (char *s)
{
    printf ("panic: %s\n", s);
}
#endif

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
