/* $Id: sbd.c,v 1.5 1999/04/22 16:35:36 chris Exp $ */
#include <pmon.h>
#include "termio.h"
#include "lr33000.h"
#include "lr33020.h"
#include "lsipr/p2681.h"

#include "sbd.h"

#ifdef MIPSEB
struct p2681info HTPdat = {(byte *) 0xbe000003};
#else
struct p2681info HTPdat = {(byte *) 0xbe000000};
#endif

extern int p2681 ();

const ConfigEntry ConfigTable[] =
{
    {(Addr) & HTPdat, 0, p2681, 1024, B9600},
    {(Addr) & HTPdat, 1, p2681, 1024, B9600},
    {0}};


int icache_size = 4*1024;
int dcache_size = 1*1024;

getmachtype ()
{
    if (hasFPU ())
      return (33050);
    else if (hasCP2 ())
      return (33020);
    icache_size = 8*1024;
    return (33000);
}


const char *
sbdgetname ()
{
#ifdef RACERX
    return "RACERX";
#else
    return "LSIPR";
#endif
}

int
sbdcpufreq ()
{
    return 25000000;
}


/* early low-level machine initialisation */
void
sbdmachinit ()
{
    memorysize = sizemem (CLIENTPC, LOCAL_MEM_SIZE);
}


void
sbdpoll ()
{
    /* poll any special devices */
}


void
sbddevinit ()
{
    /* program any local devices */
}


unsigned int
sbdenable (machtype)
{
    unsigned int sr = 0;

    switch (machtype) {
    case 33050:
	CFGREG |= CR_WBE;
	sr = SR_IBIT6 | SR_IEC;
	break;
    case 33020:
	cp2init ();
	CFGREG |= ((7 << CR_BANKSHFT) | (2 << CR_PGSZSHFT) | CR_BEN);
	sr = SR_CU2;
	break;
    }

    (void) bis_sr (sr);
    return sr;
}


sbdberrenb ()
{
    return 0;
}


sbdberrcnt ()
{
    return 0;
}


void
sbdblank ()
{
}


void
sbddisplay ()
{
}


char *
sbddbgintr (unsigned int cr)
{
    return 0;
}

char *
sbdexception (epc, cause, ra, extra, exc)
    unsigned long   epc, cause, ra, extra;
    char *exc;
{
    return 0;
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


gsignal (jb, sig)
    jmp_buf *jb;
    int sig;
{
    if (jb)
      longjmp (jb, 1);
}

/* stubs for missing network support */
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
