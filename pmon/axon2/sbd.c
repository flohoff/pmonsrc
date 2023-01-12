/* $Id: sbd.c,v 1.5 1999/04/22 16:34:54 chris Exp $ */
#include "mips.h"
#include "termio.h"
#include "pmon.h"
#include "axon2/sbd.h"
#include "axon2/m82510.h"

#ifdef SBDDISPLAYPRINT
int	sbddisplayprint;
#endif

extern int m82510 ();
const ConfigEntry ConfigTable[] =
{
    {(Addr)0xb0000000, 0, m82510, 256, B9600},
    {(Addr)0xb4000000, 0, m82510, 256, B9600},
    {0}};


/* return the current CPU type */
getmachtype ()
{
    extern int icache_size;
    switch ((Prid >> 8) & 0xff) {
    case 0x02:
	if (initial_sr & SR_CU1) 
	    return 3081;
	return 3071;		/* no fpu */
    case 0x03:
	if (initial_sr & SR_CU1) 
	    return 3081;	/* got an fpu */
	if (icache_size == 8192)
	    return 3052;	/* bigger caches */
	return 3051;
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
    return "AXON2";
}


/* return CPU clock frequency */
int
sbdcpufreq ()
{
    /* should be calculated dynamically using timer */
    return MHZ * 1000000;
}


/* very early low-level machine initialisation */
void
sbdmachinit ()
{
    memorysize = 0x80000;	/* 512Kb */
/*    memorysize = sizemem (CLIENTPC, MEM_SIZE); */
}


/* initialise any local devices (except uarts, which are handled via ConfigTable) */
void
sbddevinit ()
{
}


void
sbdpoll ()
{
    /* poll any special devices */
}


/* enable any local interrupts which we will handle */
unsigned int
sbdenable (machtype)
{
    unsigned int sr = 0;
    if (initial_sr & SR_CU1) {
	/* enable FPU interrupts */
	sr |= SR_IBIT6 | SR_IEC;
	(void) bis_sr (sr);
    }
    return sr;
}


/* control the l.e.d. blank enable */
void
sbdblank (int blankon)
{
}


#ifndef SLOWLED
/* display a message on the l.e.d. (four chars packed into one word) */
void
sbddisplay (unsigned long msg)
{
#ifdef SBDDISPLAYPRINT
    char buf[5];
    if (sbddisplayprint)
	printf ("Status: %c%c%c%c\n", msg>>24, msg>>16, msg>>8, msg);
#endif
}
#endif


/* return a string describing the current debug/panic interrupt, if any */
char *
sbddbgintr (unsigned int cr)
{
    return 0;
}


/* called on a fatal prom exception: display it on l.e.d, (if present) */
char *
sbdexception (epc, cause, ra, extra, exc)
    unsigned long epc, cause, ra, extra;
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


/* gsignal() is called when a keyboard interrupt (^C) is detected */
gsignal (jb, sig)
    jmp_buf *jb;
    int sig;
{
    if (jb)
      longjmp (jb, 1);
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
