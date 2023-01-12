/* $Id: sbd.c,v 1.11 2000/03/28 00:21:59 nigel Exp $ */
/* p4000/p4000.c */

#include <mips.h>
#include <pmon.h>
#include <termio.h>
#include <p4000/sbd.h>

extern int pmpsc();
extern void sbddisplay (unsigned long, unsigned int);

ConfigEntry     ConfigTable[] =
{
    /* p4000 has swapped mpsc ports */
    {(Addr)PHYS_TO_K1(MPSC_BASE+4), 0, pmpsc, 256, B9600},
    {(Addr)PHYS_TO_K1(MPSC_BASE+0), 1, pmpsc, 256, B9600},
    {0}};


const char *
sbdgetname ()
{
    return "P4000";
}



/* early low-level machine initialisation */
void
sbdmachinit ()
{
    sbd_rtcinit ();
    (void) *(volatile unsigned int *)PHYS_TO_K1(INT_ACKPANIC);
    if (memorysize == 0)
	memorysize = sizemem (CLIENTPC, LOCAL_MEM_SIZE);
}


void
sbddevinit ()
{
    /* program any local devices */
}


/* board-specific environment override */
void
sbd_boardenv (char *(*get)(const char *),
	       int (*set)(const char *, const char *))
{
#ifdef INET
    /* no ethernet serial rom, put default in environment */
    if (!(*get) ("ethaddr"))
	(*set) ("ethaddr", "00:40:bc:00:21:00");
#endif
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

    /* now is the chance to report any early failures */
    sbd_envreport ();

    /* clear left-over panics */
    (void) *(volatile unsigned int *)PHYS_TO_K1(INT_ACKPANIC);

    /* enable panic interrupts */
    sr = SR_IBIT7 | SR_IE;

    if (Prid == 0x0a11)
      /* Rev 1.1 R4200: disable parity errors */
      sr |= SR_DE;

    (void) bis_sr (sr);
    return sr;
}


char *
sbddbgintr (unsigned int cr)
{
    extern int cpuspeed;
    register int i;

    if (cr & CAUSE_IP7) {
	unsigned int irr0 = *(volatile unsigned int *) PHYS_TO_K1 (INT_IRR0);
	(void) *(volatile unsigned int *) PHYS_TO_K1 (INT_ACKPANIC);
	if (irr0 & IRR0_BUS)
	  return "Bus Error";
	if (irr0 & IRR0_ACFAIL)
	  return "Power Fail";
	if (irr0 & IRR0_DEBUG) {
	    /* software debounce debug button
	     * (hope no other panics arrive in the meantime)
	     */
	    do {
		(void) *(volatile unsigned int *) PHYS_TO_K1 (INT_ACKPANIC);
		for (i = cpuspeed * 1000; i > 0; i--) continue;
		irr0 = *(volatile unsigned int *) PHYS_TO_K1 (INT_IRR0);
	    } while (irr0 & IRR0_DEBUG);
	    return "Debug";
	}
    }
    return 0;
}


static void 
display (s)
    char *s;
{
    int l  = strlen (s);
    int i, d;

    /* scroll message across display */
    for (i = -4; i < l; i++) {
	unsigned long v = 0;
	for (d = i; d < i + 4; d++) {
	    v <<= 8;
	    v |= (d < 0 || d >= l) ? ' ' : (s[d] & 0xff);
	}
	sbddisplay (v, 0);
	sbdmsdelay (250);
    }
}


static void
xdisplay (s, x)
    char *s;
    unsigned int x;
{
    char buf[40];
    sprintf (buf, "%s=%04x.%04x", s, x >> 16, x & 0xffff);
    display (buf);
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
	  sprintf (excbuf, "PMON %s exception", getexcname (cause & CAUSE_EXCMASK));
	exc = excbuf;
    }

    sbdblank (0);		/* display on */
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
    SBD_DISPLAY ("    ", 0);
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
    extern int mips_icache_size, mips_dcache_size;
    switch ((Prid >> 8) & 0xff) {
    case 0x04:
	return (mips_icache_size == 16384) ? 4400 : 4000;
    case 0x0a:
	return 4200;
    case 0x20:
	return 4600;
    case 0x21:
	return 4700;
    default:
	return 0;
    }
}

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
