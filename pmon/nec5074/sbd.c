/*
 * nec5074/sbd.c: board support for NEC DDB-Vrc5074
 * Copyright (c) 1999	Algorithmics Ltd
 */

#include "mips.h"
#include "termio.h"
#include "pmon.h"

#include "mips/prid.h"
#include "sbd.h"

#include "vrc5074.h"

extern int ns16550 ();
extern int ns16550i ();
const ConfigEntry ConfigTable[] =
{
    {(Addr)PHYS_TO_K1(UART0_BASE), 0, ns16550, 256, B9600},
    {(Addr)PHYS_TO_K1(UART1_BASE), 0, ns16550, 256, B9600},
    {(Addr)PHYS_TO_K1(UARTI_BASE), 0, ns16550i, -256, B9600},
    {0}
};


/* return the current CPU type */
unsigned int 
getmachtype (void)
{
    return 5000;
}


/* return name of this board */
const char *
sbdgetname (void)
{
    return "NEC DDB-Vrc5074";
}


/* very early low-level machine initialisation */

void
sbdmachinit (void)
{
    /* get memory size */
    if (memorysize == 0) {
	volatile struct vrc5074 *n4 = PA_TO_KVA1 (VRC5074_BASE);
	unsigned int sz;
	sz = n4->n4_sdram0 & N4_PDAR_SIZE_MASK;
	memorysize = 0x200000 << (N4_PDAR_2MB - sz);
    }

    /* disable error interrupts until sbdenable() */
    (void) bic_sr (SR_IMASK | SR_IE);
}


/* initialise any local devices (except uarts, which are handled via ConfigTable) */
void
sbddevinit (void)
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
    volatile struct vrc5074 *n4 = PA_TO_KVA1 (VRC5074_BASE);
    unsigned int sr = 0;

#if defined(FLASH) || defined(NVENV)
    /* now's a good time to tell the poor loser about his zapped flash */
    extern const char *_sbd_envinit (void);
    const char *err;
    if (err = _sbd_envinit ())
	printf ("\nWARNING: environment: %s\n", err);
#endif

    /* clear any pending interrupts */
    n4->n4_intclr = ~0; wbflush();

#if 0
    /* enable error interrupts on Intr4 */
    sr = SR_IBIT7 | SR_IE;
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


/* return a string describing the current debug/panic interrupt, if any */
char *
sbddbgintr (unsigned int cr)
{
    *(unsigned long *)PA_TO_KVA1(LED_BASE) = 0xdddddddd;
    if (cr & CAUSE_IP7) {
	volatile struct vrc5074 *n4 = PA_TO_KVA1(VRC5074_BASE);
	unsigned long long intstat1 = n4->n4_intstat1;
	static char msg[64];

	if (intstat1 & N4_INTSTAT1_IL4(N4_INT_MCE)) {
	    n4->n4_intclr = N4_INTCLR_DEV(N4_INT_MCE);
	    sprintf (msg, "Memory Check Error (chk=0x%08llx)", n4->n4_chkerr);
	    return msg;
	}
	if (intstat1 & N4_INTSTAT1_IL4(N4_INT_CNTD)) {
	    n4->n4_intclr = N4_INTCLR_DEV(N4_INT_CNTD);
	    return "No Target Decode";
	}
	if (intstat1 & N4_INTSTAT1_IL4(N4_INT_LBRT)) {
	    n4->n4_intclr = N4_INTCLR_DEV(N4_INT_LBRT);
	    return "Local Bus Timeout";
	}
	if (intstat1 & N4_INTSTAT1_IL4(N4_INT_PCIE)) {
	    unsigned long long addr;
	    const char *pcimsg;

	    n4->n4_intclr = N4_INTCLR_DEV(N4_INT_PCIE);
	    switch (n4->n4_pcictrl & N4_PCICTRL_ERRTYPE_MASK) {
	    case 0:
		pcimsg = "no"; break;
	    case N4_PCICTRL_ERRTYPE_TABORT:
		pcimsg = "target abort"; break;
	    case N4_PCICTRL_ERRTYPE_MABORT:
		pcimsg = "master abort"; break;
	    case N4_PCICTRL_ERRTYPE_RETRYLIM:
		pcimsg = "retry limit"; break;
	    case N4_PCICTRL_ERRTYPE_RDPERR:
		pcimsg = "read parity"; break;
	    case N4_PCICTRL_ERRTYPE_WRPERR:
		pcimsg = "write parity"; break;
	    case N4_PCICTRL_ERRTYPE_DTIMEX:
		pcimsg = "deferred timeout"; break;
	    default:
		pcimsg = "unknown"; break;
	    }

	    addr = n4->n4_pcierr;
	    n4->n4_pcierr = 0;

	    sprintf (msg, "PCI %s error (addr=0x%llx)", pcimsg, addr);
	    return msg;
	}
	if (intstat1 & N4_INTSTAT1_IL4(N4_INT_PCIS)) {
	    n4->n4_intclr = N4_INTCLR_DEV(N4_INT_PCIS);
	    return "PCI SERR#";
	}
	sprintf (msg, "Unknown Vrc5074 (ist1[4]=0x%04llx)", 
		 (intstat1 & N4_INTSTAT1_IL4_MASK) >> N4_INTSTAT1_IL4_SHIFT);
	return msg;
    }
    return 0;
}


/* called on a fatal prom exception: display it on l.e.d, (if present) */
char *
sbdexception (epc, cause, ra, extra, exc)
    unsigned long epc, cause, ra, extra;
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
    return exc
;
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


#ifdef RTC
time_t __time (void)
{
    return (sbd_gettime ());
}
#endif
#endif
