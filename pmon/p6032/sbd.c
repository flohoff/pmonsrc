/*
 * p6032/sbd.c: board support for Algorithmics P-6032
 *
 * Copyright (c) 2001, Algorithmics Ltd.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the "Free MIPS" License Agreement, a copy of 
 * which is available at:
 *
 *  http://www.algor.co.uk/ftp/pub/doc/freemips-license.txt
 *
 * You may not, however, modify or remove any part of this copyright 
 * message if this program is redistributed or reused in whole or in
 * part.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * "Free MIPS" License for more details.  
 */

#include <mips.h>
#include <pmon.h>
#include <termio.h>
#include <mips/prid.h>

#include "sbd.h"
#include "rtc.h"
#include "i82371eb.h"

#include "pcivar.h"
#include "pcireg.h"

static char * const _bonito = PA_TO_KVA1(BONITO_BASE);
extern int ns16550();


#define MAXDISPLAYCOLS	8

static int displaycols;
static int displayrows;

ConfigEntry	ConfigTable[] =
{
    {(Addr)PHYS_TO_K1(UART0_BASE), 0, ns16550, 256, B9600},
    {(Addr)PHYS_TO_K1(UART1_BASE), 0, ns16550, 256, B9600},
    {0}
};

getmachtype ()
{
    extern int mips_icache_size;
    extern int mips_scache_size;
    switch (Prid >> 8) {
    case PRID_R4000:
	return 4000;
    case PRID_R4300:
	return 4300;
    case PRID_R4100:
	return 4100;
    case PRID_RC6457X:
	return 64574;
    case PRID_R4600:
	return 4600;
    case PRID_R4700:
	return 4700;
    case PRID_R4650:
	return 4640;		/* could be 4650? */
    case PRID_R5000:
	return 5000;
    case PRID_RM7000:
	return 7000;
    case PRID_RM52XX:
	/* blech */
	if (mips_scache_size > 0)
	    return 5270;
	return (mips_icache_size == 32*1024) ? 5231 : 5230;
	if (mips_icache_size == 32*1024)
	    return (5261);
	return 5260;
    case PRID_RC6447X:
	return 64474;
    case PRID_R5400:
	/* who knows which ? */
	return 5432;
/*	return 5464;*/
    case PRID_R5500:
	return 5500;
    case PRID_JADE:
	return 4032;		/* XXXX think of a number */
    default:
	return 0;
    }
}

int
sbd_pcbrev()
{
    unsigned int gpi = (inb (PM_PORT + I82371_PM_GPIREG2) << 16)
	| (inb (PM_PORT + I82371_PM_GPIREG1) << 8)
	| (inb (PM_PORT + I82371_PM_GPIREG0));
    /* printf ("sbd_pcbrev: gpi=0x%x, pcbrev=%d\n", gpi, (gpi >> 17) & 7); */
    return (gpi >> 17) & 7;
}

int
sbd_modrev()
{
    unsigned int gpi = (inb (PM_PORT + I82371_PM_GPIREG2) << 16)
	| (inb (PM_PORT + I82371_PM_GPIREG1) << 8)
	| (inb (PM_PORT + I82371_PM_GPIREG0));
    /* printf ("sbd_modrev: gpi=0x%x, pcbrev=%d\n", gpi, (gpi >> 13) & 15); */
    return (gpi >> 13) & 15;
}

int
sbd_boardtype (void)
{
    return (sbd_pcbrev() & 4) ? 64 : 32;
}

unsigned int
sbd_switches (void)
{
    unsigned short cpldrev = *(unsigned short *)(PA_TO_KVA1(CPLD_BASE + CPLD_SWITCHES));
    unsigned short cpldswitches;

    if (cpldrev < 2)
	return *(unsigned short *)(PA_TO_KVA1(CPLD_BASE + CPLD_SWITCHES_PREREV2));
    else
	return *(unsigned short *)(PA_TO_KVA1(CPLD_BASE + CPLD_SWITCHES));
}

/* return name of this board */
const char *
sbdgetname ()
{
    static char board[30];

    sprintf (board, "Algorithmics P-60%d Rev%c.%d",
	     sbd_boardtype(),
	     'A' + (sbd_pcbrev() & 3),
	     sbd_modrev());
    return board;
}


/* very early low-level machine initialisation */
void
sbdmachinit ()
{

#ifdef RTC
    /* set RTC periodic interrupt frequency */
    _rtc_bic (RTC_STATUSA, RTC_RATE_MASK);
    _rtc_bis (RTC_STATUSA, RTC_RATE);
    /* enable periodic interrupts */

    _rtc_bis (RTC_STATUSB, RTCSB_PIE);
#endif

    sbd_dispinit ();

    if (memorysize == 0) {
	/* size memory (look for sbdreset's computed value) */
	memorysize = *(unsigned int *) PA_TO_KVA1 (0x180-4);
    }

    /* disable all until sbdenable() */
    (void) bic_sr (SR_IMASK | SR_IE);
}


/* initialise any local devices (except uarts, which are handled via ConfigTable) */
void
sbddevinit (void)
{
}


/* board-specific environment override */
void
sbd_boardenv (char *(*get)(const char *),
	       int (*set)(const char *, const char *))
{
    /* set memory size unless it has already been defined */
    if (!(*get)("memsize")) {
	char buf[11];
	sprintf (buf, "0x%08x", *(unsigned int *) PA_TO_KVA1 (0x180-4));
	(*set) ("memsize", buf);
    }
}

void
sbdpoll (void)
{
#if !defined(INET) && defined(RTC)
    /* poll any special devices */
    static int on = 0;
    static int count = 0;

    if (_rtc_get (RTC_INTR) & RTC_INTRF) {
	if (++count >= RTC_HZ/2) {
	    count = 0;
	    display_flash (on = !on);
	}
    }
#endif
}


/* enable any local interrupts which we will handle */
unsigned int
sbdenable (unsigned int machtype)
{
    unsigned int sr = 0;

#if defined(_SBD_FLASHENV) || defined(_SBD_RTCENV)
    /* now's a good time to tell the poor loser about his zapped flash */
    extern const char *_sbd_envinit (void);
    const char *err;
    if (err = _sbd_envinit ())
	printf ("\nWARNING: environment: %s\n", err);
#endif

    /* enable debug & bonito interrupts */
    sr = SR_IBIT6 | SR_IBIT4 | SR_IBIT3 | SR_IE;
    (void) bis_sr (sr);

    if (Prid == 0x5413)
	sr |= SR_DE;		/* Disable cache errors for Vr5432 Rev1.3 */
    return sr;
}


sbd_dispinit ()
{
    /* initialise display */
    display_init ();

    display_size (&displayrows, &displaycols);
    if (displaycols > MAXDISPLAYCOLS)
	displaycols = MAXDISPLAYCOLS;
}


/* control the l.e.d. blank enable */
void
sbdblank (int blankon)
{
    display_flash (!blankon);
}



/* display a message (four chars packed into one word),
   for very low-level code. */
void
sbddisplay (unsigned long msg, unsigned int x)
{
    char mbuf[MAXDISPLAYCOLS+1], *mp = mbuf;
    int i;

    sbdblank (0); /* display on */

#if 0
    /* centre it */
    i = (displaycols - 4) / 2;
    while (i-- > 0)
	*mp++ = ' ';
#endif
    
    /* split up into characters */
    for (i = 3; i >= 0; i--, msg >>= 8)
	mp[i] = msg & 0xff;
    mp[4] = '\0';

    display_message (0, mbuf);
}


/* display a long message on the display, scrolling if necessary. */
void
sbdmessage (int line, const char *msg)
{
    int len = strlen (msg);

    display_clear ();
    sbdblank (0); /* display on */
    if (len <= displaycols) {
	/* plonk short message straight onto screen */
	display_message (line, msg);
    }
    else {
	/* scroll long message across screen */
	char mbuf[MAXDISPLAYCOLS+1];
	int start, i;
	for (start = -displaycols+1; start <= len - displaycols; start++) {
	    for (i = start; i < start + displaycols; i++)
		mbuf[i - start] = (i < 0 || i >= len) ? ' ' : msg[i];
	    mbuf[displaycols] = '\0';
	    display_message (line, mbuf);
	    msdelay(100);	/* 1/10 sec delay */
	}
    }
}

/* display a message and then pause so that it can be read */
static void 
display (const char *msg)
{
    sbdmessage (0, msg);
    msdelay (1000);		/* 1 sec delay */
}


char *
sbddbgintr (unsigned int cr)
{
    if (cr & CAUSE_IP6) {
	do {
	    msdelay (100);
	} while (get_cause() & CAUSE_IP6);
	return "Debug";
    }
    return 0;
}


/* display a register name and its value, then pause */
static void
xdisplay (char *s, unsigned int x)
{
    char buf[40];

    sprintf (buf, "%s=%08x", s, x);
#if 0
    printf ("%s\n", buf);
#endif
    display (buf);
}


char *
sbdexception (epc, cause, ra, extra, exc)
    unsigned long   epc, cause, ra, extra;
    char *exc;
{
    static char excbuf[80];
    char *s;

    if (!exc) {
	if ((cause & CAUSE_EXCMASK) == CEXC_INT && (s = sbddbgintr (cause)))
	    sprintf (exc = excbuf, "%s interrupt", s);
        else if (cause == CEXC_CP2) 
	    sprintf (excbuf, "Cache exception");
	else
	  sprintf (excbuf, "%s exception", 
		   getexcname (cause & CAUSE_EXCMASK));
	exc = excbuf;
    }
    display  (exc);
    if (cause != CEXC_CP2)
	xdisplay ("Cause", cause);
    xdisplay ("EPC", epc);
    xdisplay ("RA", ra);
    switch (cause & CAUSE_EXCMASK) {
    case CEXC_MOD:
    case CEXC_TLBL:
    case CEXC_TLBS:
    case CEXC_ADEL:
    case CEXC_ADES:
	xdisplay ("BadVa", extra);
	break;
    case CEXC_CP2:
	xdisplay ("CErr", extra);
	break;
    }
    display ("");
    return exc;
}




#if !defined(FLASH) && !defined(NVENV)

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
#ifdef INET
    (*func) ("bootp", "pri");
#endif
}
#endif

#if !defined(FLASH)
void
sbd_flashinfo (void **flashbuf, int *flashsize)
{
    *flashbuf = (void *)0;
    *flashsize = 0;
}


int
sbd_flashprogram (unsigned char *data, unsigned int size, unsigned int offset)
{
    printf ("This port does not support FLASH\n");
    return -1;
}
#endif



/*
 * floating point emulator stubs
 */

int
_fpstatesz (void)
{
    return 0;
}

int
_fpstate (void)
{
    return -1;
}

int
cop1 (void)
{
    printf ("\ncop1 called\n");
    cliexit ();
}


#ifndef INET
/* gsignal() is called when a keyboard interrupt (^C) is detected */
gsignal (jmp_buf *jb, int sig)
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
#endif

/* print board specific information */
void sbdmachinfo (void)
{
    {
#ifndef BONITO_BUILDINFO
#define BONITO_BUILDINFO  BONITO(BONITO_REGBASE + 0x64)
#endif
	char buf[33];
	char *s = buf;
	char c;
	int i;
	unsigned int bonrev = PCI_REVISION(BONITO_PCICLASS);
	for (i = 31; i >= 0; i--) {
	  BONITO_BUILDINFO = i << 8;
	  c = BONITO_BUILDINFO & 0xff;
	  if (c) {
	    *s++ = c;
	  }
	}
	*s = '\0';
	printf ("Bonito revision: %s %x.%x %s\n",
		(bonrev & 0x80) ? "FPGA" : "ASIC",
		(bonrev >> 4) & 0x7, bonrev & 0xf,
		s != buf ? buf : "");
    }
    {
	unsigned int cpldrev = *(unsigned int *)PA_TO_KVA1(CPLD_BASE+CPLD_REVISION);
	printf ("CPLD revision %d.%d\n", (cpldrev >> 5) & 0x7, cpldrev & 0x1f);
    }
    
}

#ifndef mips_getcount
#define mips_getcount() \
({ \
  unsigned long __r; \
  __asm__ __volatile ("mfc0 %0,$9" : "=r" (__r)); \
  __r; \
})
#endif

void
_sbd_nsdelay (unsigned long ns)
{
    static unsigned int nspercount;
    unsigned long start, count;

    if (nspercount == 0) {
	extern unsigned long sbdpipefreq();
	nspercount = 1000000000 / sbdpipefreq ();

#ifdef _SBD_CLK_DIV
	nspercount = (nspercount * _SBD_CLK_DIV) / _SBD_CLK_MULT;
#else
	if (getmachtype() == 4100)
	    /* R4100 counter runs at pipeline frequency / 4 */
	    nspercount *= 4;
	else
	    /* R4x00 counter runs at pipeline frequency / 2 */
	    nspercount *= 2;
#endif
    }

    /* calculate number of counter ticks (rounding up) */
    count = (ns + nspercount / 2) / nspercount;

    /* discount cost of function call & divide (~32 cycles) */
    if (count < 32)
	return;
    count -= 32;

    start = mips_getcount ();
    while ((mips_getcount () - start) < count)
	continue;
}

/*
 * Bonito i/o buffer cache handling
 *
 * The i/o buffer cache snoops on uncached cycles by the MIPS cpu, so
 * we don't need to do anything for uncached CPU acceses (e.g. command
 * and status descriptors). But we do have to explicitly write-back
 * and invalidate buffer cache lines if we have used cached CPU accesses.
 */

/* FIXME these definitions should be in bonito.h! */
#define CACHECMD_INVAL	0
#define CACHECMD_WBINV	1
#define CACHECMD_RDTAG	2
#define CACHECMD_WQFLUSH 3

#define TAG_LOCK	0x80000000
#define TAG_WBACK	0x40000000
#define TAG_PFPEND	0x20000000
#define TAG_PEND	0x10000000
#define TAG_MOD		0x08000000
#define TAG_PFDVAL	0x04000000
#define TAG_DVAL	0x02000000
#define TAG_AVAL	0x01000000
#define TAG_ADDR	0x00ffffff


/*
 * Execute an i/o buffer cache command 
 */
 
static void
_bonito_iobc_cmd (unsigned int cmd, unsigned int line)
{
    unsigned int ctrl;
	
    ctrl = (cmd << BONITO_PCICACHECTRL_CACHECMD_SHIFT)
	| (line << BONITO_PCICACHECTRL_CACHECMDLINE_SHIFT);
    
    BONITO_PCICACHECTRL = ctrl;
    BONITO_PCICACHECTRL = ctrl | BONITO_PCICACHECTRL_CMDEXEC;
    while (BONITO_PCICACHECTRL & BONITO_PCICACHECTRL_CMDEXEC)
	continue;
    BONITO_PCICACHECTRL = ctrl;
}


/*
 * Writeback and invalidate any i/o buffer cache lines which overlap
 * the specified address range. 
 */
 
void
_bonito_iobc_wbinv (unsigned int pa, size_t nb)
{
    unsigned int line;
    unsigned int tag;
    
    for (line = 0; line < 4; line++) {
	_bonito_iobc_cmd (CACHECMD_RDTAG, line);
	tag = BONITO_PCICACHETAG;
	if (tag & TAG_AVAL) {
	    unsigned int tagaddr = (tag & TAG_ADDR) << 5;
	    if (nb == ~(size_t)0 ||
		(tagaddr < pa + nb && tagaddr + 32 > pa))
		_bonito_iobc_cmd (CACHECMD_WBINV, line);
	}
    }
    _bonito_iobc_cmd (CACHECMD_WQFLUSH, 0);
}


/*
 * Invalidate (no writeback!) any i/o buffer cache lines which overlap
 * the specified address range. 
 */
 
void
_bonito_iobc_inval (unsigned int pa, size_t nb)
{
    unsigned int line;
    unsigned int tag;
    
    for (line = 0; line < 4; line++) {
	_bonito_iobc_cmd (CACHECMD_RDTAG, line);
	tag = BONITO_PCICACHETAG;
	if (tag & TAG_AVAL) {
	    unsigned int tagaddr = (tag & TAG_ADDR) << 5;
	    if (nb == ~(size_t)0 ||
		(tagaddr < pa + nb && tagaddr + 32 > pa))
		_bonito_iobc_cmd (CACHECMD_INVAL, line);
	}
    }
}


void
_bonito_clean_dcache (void *addr, size_t nb)
{
    if (IS_KVA0 (addr)) {
	_bonito_iobc_wbinv (KVA_TO_PA (addr), nb);
	mips_clean_dcache (addr, nb);
    }
}


void
_bonito_inval_dcache (void *addr, size_t nb)
{
    if (IS_KVA0 (addr)) {
	/* when invalidating cpu data cache, we still need to make
	   sure that the i/o buffer cache has been written back. */
	_bonito_iobc_wbinv (KVA_TO_PA (addr), nb);
#if 0
	mips_clean_dcache_nowrite (addr, nb);
#else
	mips_clean_dcache (addr, nb);
#endif
    }
}


void _mon_putc (char c)
{
    iFunc *fp = ConfigTable[0].handler;
    const Addr dp = ConfigTable[0].devinfo;
    const int chan = ConfigTable[0].chan;

    while ((*fp) (OP_TXRDY, dp, chan, 0) == 0)
	continue;
    (*fp) (OP_TX, dp, chan, c);
}


void _mon_puts (const char *s)
{
    char c;

    while (c = *s++) {
	if (c == '\n')
	    _mon_putc ('\r');
	_mon_putc (c);
    }
}

void
sbd_off (int argc, char **argv)
{
    _apc_bis (RTC_BANK2_APCR1, APCR1_SOC);
    wbflush ();
    sbddelay (100000);		/* 100ms */
    printf ("power-off failed\n");
}

void *
ioport_map (unsigned int port)
{
    return PA_TO_KVA1 (ISAPORT_BASE(port));
}


unsigned char
inb (unsigned int port)
{
#if #endian(big) && defined(P6032)
    return *(volatile unsigned char *) PA_TO_KVA1 (ISAPORT_BASE(port^3));
#else
    return *(volatile unsigned char *) PA_TO_KVA1 (ISAPORT_BASE(port));
#endif
}

unsigned short
inw (unsigned int port)
{
#if #endian(big) && defined(P6032)
    return *(volatile unsigned short *) PA_TO_KVA1 (ISAPORT_BASE(port^2));
#else
    return *(volatile unsigned short *) PA_TO_KVA1 (ISAPORT_BASE(port));
#endif
}

unsigned long
inl (unsigned int port)
{
    return *(volatile unsigned long *) PA_TO_KVA1 (ISAPORT_BASE(port));
}

void
outb (unsigned int port, unsigned char val)
{
#if #endian(big) && defined(P6032)
    *(volatile unsigned char *) PA_TO_KVA1 (ISAPORT_BASE(port)^3) = val;
#else
    *(volatile unsigned char *) PA_TO_KVA1 (ISAPORT_BASE(port)) = val;
#endif
    mips_wbflush ();
}

void
outw (unsigned int port, unsigned short val)
{
#if #endian(big) && defined(P6032)
    *(volatile unsigned short *) PA_TO_KVA1 (ISAPORT_BASE(port^2)) = val;
#else
    *(volatile unsigned short *) PA_TO_KVA1 (ISAPORT_BASE(port)) = val;
#endif
    mips_wbflush ();
}

void
outl (unsigned int port, unsigned long val)
{
    *(volatile unsigned long *) PA_TO_KVA1 (ISAPORT_BASE(port)) = val;
    mips_wbflush ();
}
