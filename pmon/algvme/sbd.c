/* $Id: sbd.c,v 1.6 2000/03/28 00:21:11 nigel Exp $ */
/* algvme/sbd.c */

#include <mips.h>
#include <pmon.h>
#include <termio.h>
#include <algvme/sbd.h>
#include <algvme/vac068.h>

extern int pvacser();

const ConfigEntry ConfigTable[] =
{
    /* vme4000 has vac ports only */
    {(Addr)1, 0, pvacser, 1024, B9600},
    {(Addr)1, 1, pvacser, 1024, B9600},
    {0}};

sbdmachinit ()
{
    sbd_rtcinit ();
}


void
sbddevinit ()
{
    /* initialise vic/vac etc */
}


void
sbdpoll ()
{
    /* poll any special devices */
}


unsigned int
sbdimask ()
{
    return 0;
}


const char *
sbdgetname ()
{
#if defined(VME4000)
    return "SB4000";
#elif defined(VME3000)
    return "VME3000";
#elif defined(SB3000)
    return "SB3000";
#elif defined(SL3000)
    return "SL3000";
#else
    return "???";
#endif
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

#ifndef SL3000
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
#endif
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
#ifdef R4000
    extern int mips_icache_size, mips_dcache_size;
    switch ((Prid >> 8) & 0xff) {
    case 0x04:
	return (mips_icache_size == 8192) ? 4000 : 4400;
    case 0x0a:
	return 4200;
    case 0x20:
	return 4600;
    default:
	return 0;
    }
#else
    if (hasFPU ())
      return (3010);
    else
      return (3000);
#endif
}

/*
 * flash support stub
 */

void
sbd_flashinfo (void **flashbuf, int *flashsize)
{
    *flashbuf = (void *)0;
    *flashsize = 0;
    return 0;
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
