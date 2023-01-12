/*
 * devaz/sbd.c: board support for Algorithmics DEVA-0
 * Copyright (c) 1999	Algorithmics Ltd
 */

#include <mips.h>
#include <pmon.h>
#include <termio.h>
#include <devaz/sbd.h>
#include <mips/prid.h>

#include "pcivar.h"
#include "pcireg.h"

static char * const _bonito = PA_TO_KVA1(BONITO_BASE);
extern int ns16550();


ConfigEntry	ConfigTable[] =
{
    {(Addr)PHYS_TO_K1(UART0_BASE), 0, ns16550, 256, B38400},
    {(Addr)PHYS_TO_K1(UART1_BASE), 0, ns16550, 256, B38400},
    {0}
};

getmachtype ()
{
    extern int mips_icache_size;
    switch (Prid >> 8) {
    case PRID_R4300:
	return 4300;
    case PRID_R4100:
	return 4100;
    case PRID_R4650:
	return 4640;
    case PRID_RM52XX:
	return (mips_icache_size == 32*1024) ? 5231 : 5230;
    case PRID_RC6447X:
	return 64474;
    case PRID_R5400:
	return 5432;
    default:
	return 0;
    }
}

/* return name of this board */
const char *
sbdgetname ()
{
    return "Algorithmics DEVA-0";
}


/* very early low-level machine initialisation */
void
sbdmachinit ()
{
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
}


void
sbdpoll (void)
{
    /* poll any special devices */
}


/* enable any local interrupts which we will handle */
unsigned int
sbdenable (unsigned int machtype)
{
    unsigned int sr = 0;

#if defined(FLASH) || defined(NVENV)
    /* now's a good time to tell the poor loser about his zapped flash */
    extern const char *_sbd_envinit (void);
    const char *err;
    if (err = _sbd_envinit ())
	printf ("\nWARNING: environment: %s\n", err);
#endif

#if 0
    /* enable bonito interrupts */
    sr = SR_IBIT3 | SR_IBIT2 | SR_IE;
    (void) bis_sr (sr);
#endif

    return sr;
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


char *
sbddbgintr (unsigned int cr)
{
    if (cr & CAUSE_IP7) {
    }
    return 0;
}


char *
sbdexception (epc, cause, ra, extra, exc)
    unsigned long   epc, cause, ra, extra;
    char *exc;
{
    static char excbuf[80];
    char *s;

#if 0
    /* write exception to PROM, encoding data in address */
    register volatile unsigned char *prom = (void *) 0xbfdff00;
#define pByte(x) prom[(x) & 0xff] = 0
#define pWord(x) {pByte(x); pByte(x>>8); pByte(x>>16); pByte(x>>24);}
    pWord(cause);
    pWord(epc);
    pWord(extra);
#endif

    if (!exc) {
	if ((cause & CAUSE_EXCMASK) == CEXC_INT && (s = sbddbgintr (cause)))
	  sprintf (exc = excbuf, "%s interrupt", s);
    }
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
    unsigned int rev = PCI_REVISION(BONITO_PCICLASS);
    printf ("Bonito revision: %s %x.%x\n",
	    (rev & 0x80) ? "FPGA" : "ASIC",
	    (rev >> 4) & 0xf, rev & 0xf);
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
