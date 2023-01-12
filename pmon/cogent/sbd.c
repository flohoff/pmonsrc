/* $Id: sbd.c,v 1.8 2000/03/28 00:21:20 nigel Exp $ */
/* cogent/sbd.c */

#include <mips.h>
#include <pmon.h>
#include <termio.h>
#include <cogent/sbd.h>
#include <cogent/cm1629.h>

extern int ns16550();

ConfigEntry     ConfigTable[] =
{
    {(Addr)PHYS_TO_K1(IOCHAN0_BASE), 0, ns16550, 256, B9600},
    {(Addr)PHYS_TO_K1(IOCHAN1_BASE), 0, ns16550, 256, B9600},
    {0}};


const char *
sbdgetname ()
{
    return "COGENT";
}



/* early low-level machine initialisation */
void
sbdmachinit ()
{
    sbd_rtcinit ();

    if (memorysize == 0)
	memorysize = sizemem (CLIENTPC, LOCAL_MEM_SIZE);

    fpanel_init ();
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
sbdmessage (int line, const char *msg)
{
    extern void lcd_display (int, const char *);
    int len = strlen (msg);

    if (len <= 16) {
	/* plonk short message straight onto screen */
	lcd_display (line, msg);
    }
    else {
	/* scroll long message across screen */
	char mbuf[17];
	int start, i;
	for (start = -15; start <= len - 16; start++) {
	    for (i = start; i < start + 16; i++)
		mbuf[i - start] = (i < 0 || i >= len) ? ' ' : msg[i];
	    mbuf[17] = '\0';
	    lcd_display (line, mbuf);
	    sbddelay (250000);		/* 0.25 sec delay */
	}
    }
}


static void 
display (const char *msg)
{
    sbdmessage (1, msg);
}


static void
xdisplay (char *s, unsigned int x)
{
    char buf[40];
    sprintf (buf, "%s=%04x.%04x", s, x >> 16, x & 0xffff);
    display (buf);
    sbddelay (1000000);		/* 1 sec delay */
}


/* display a message on the l.e.d. (four chars packed into one word) */
void
sbddisplay (unsigned long msg, int x)
{
    char mbuf[16];

    sprintf (mbuf, "Status: %c%c%c%c", msg>>24, msg>>16, msg>>8, msg>>0);
    sbdmessage (1, mbuf);
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
