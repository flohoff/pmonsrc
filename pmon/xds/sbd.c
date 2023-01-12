/* $Id: sbd.c,v 1.7 2000/03/28 00:22:23 nigel Exp $ */
/* xds/sbd.c */

#include <mips.h>
#include <pmon.h>
#include <termio.h>
#include <xds/sbd.h>

#ifdef VTTY
extern int pvtty();
#endif

#ifdef PTTY
extern int pptty();
#endif

const ConfigEntry     ConfigTable[] =
{
#ifdef VTTY
    {(Addr)PHYS_TO_K1(IOCHAN0_BASE), 0, pvtty, 256, B9600},
    {(Addr)PHYS_TO_K1(IOCHAN1_BASE), 1, pvtty, 256, B9600},
    {(Addr)PHYS_TO_K1(IOCHAN2_BASE), 2, pvtty, 256, B9600},
    {(Addr)PHYS_TO_K1(IOCHAN3_BASE), 3, pvtty, 256, B9600},
#endif
#ifdef PTTY
    {(Addr)PHYS_TO_K1(SRAM_BASE+0x100000/2-16), 0, pptty, 256, B9600},
#endif
    {0}};


const char *
sbdgetname ()
{
    return "XDS";
}



/* early low-level machine initialisation */
void
sbdmachinit ()
{
    if (memorysize == 0)
	memorysize = sizemem (CLIENTPC, LOCAL_MEM_SIZE);
}


int
sbdpipefreq (void)
{
    return (MHZ*2);
}

/* control the l.e.d. blank enable */
void
sbdblank (int blankon)
{
}

void
sbddevinit ()
{
    /* program any local devices */
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

    /* enable panic interrupts */
    sr = SR_IE;

    if (Prid == 0x0a11)
      /* Rev 1.1 R4200: disable parity errors */
      sr |= SR_DE;

    (void) bis_sr (sr);
    return sr;
}


char *
sbddbgintr (unsigned int cr)
{
    return 0;
}


void
sbdmessage (const char *msg)
{
#if defined(PTTY) && defined(PRINTMESSAGE)
    const ConfigEntry *cp = &ConfigTable[0];
    while (*msg) {
	while (pptty(OP_TXRDY, 0xa707fff0, 0) == 0)
	    continue;
	pptty(OP_TX, 0xa707fff0, 0, *msg++);
    }
#endif
}


static void 
display (const char *msg)
{
    sbdmessage (msg);
}


static void
xdisplay (char *s, unsigned int x)
{
    char buf[40];
    sprintf (buf, "%s=%04x.%04x\n", s, x >> 16, x & 0xffff);
    display (buf);
}


/* display a message on the l.e.d. (four chars packed into one word) */
void
sbddisplay (unsigned long msg, int x)
{
    char mbuf[16];
    sprintf (mbuf, "Status: %c%c%c%c\n", msg>>24, msg>>16, msg>>8, msg>>0);
    sbdmessage (mbuf);
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
        if (cause == CEXC_CACHE) 
	  sprintf (excbuf, "\nUncorrectable Cache Error");
	else
	  sprintf (excbuf, "\nPMON %s exception", getexcname (cause & CAUSE_EXCMASK));
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
    extern int icache_size, dcache_size;
    switch ((Prid >> 8) & 0xff) {
    case 0x04:
	return (icache_size == 16384) ? 4400 : 4000;
    case 0x0a:
	return 4200;
    case 0x0b:
	return 4300;
    case 0x20:
	return 4600;
    default:
	return 0;
    }
}



unsigned int
sbd_switches (void)
{
    return (0xff & *VME_REG(PHYS_TO_K1(VME_CONF)));
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
