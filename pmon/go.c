/* $Id: go.c,v 1.15 2001/10/31 11:48:37 chris Exp $ */

#include "sys/types.h"
#include "mips/cpu.h"
#include "watchpoint.h"

#include "mips.h"
#include "pmon.h"
#include "stdio.h"
#include "termio.h"
#include "set.h"
#include "string.h"

#include "setjmp.h"

/* forward declarations */
static int	setTrcbp (word, int);
static int	is_break_point (word);
static int	is_validpc (word);
static void	addpchist (word);

int             trace_mode;
unsigned long   trace_count;
int             trace_verbose;
int             trace_invalid;
int             trace_over;
int             trace_bflag;
int             trace_cflag;

char            clientcmd[LINESZ];
char           *clientav[MAX_AC];
int		clientac;

extern int      clkdat;

#define STOPMAX 10
Stopentry       stopval[STOPMAX];

#define PCHISTSZ  200
unsigned long   pchist_d[PCHISTSZ + 1];
int             pchist_ip, pchist_op;

jmp_buf	gobuf;
int	gobufvalid;

/* set client's a0 (argc) & a1 (argv) & a2 (envp) */
void
initstack (ac, av, addenv)
    int ac;
    char **av;
    int addenv;
{
    char **vsp, *ssp;
    int ec, slen, vlen, stklen, i;
    long nsp;

    /* find total string length in arg vector (including nulls) */
    slen = 0;
    for (i = 0; i < ac; i++)
	slen += strlen(av[i]) + 1;

    /* find total string length in env vector (including nulls) */
    if (addenv)
	envsize (&ec, &slen);
    else
	ec = 0;

    /* round length to word boundary */
    slen = (slen + 3) & ~3;
    
    /* length of vector arrays (incluing terminating null pointers */
    vlen = (ac + ec + 2) * sizeof (char *);

    /* total length of stack, rounded to dword boundary */
    stklen = ((vlen + slen) + 7) & ~7;

    /* allocate stack */
    nsp = Gpr[29] - stklen;

    /* set $a0 = argc, $a1 = argv, $a2 = envp */
    Gpr[4] = ac;
    Gpr[5] = nsp;
    Gpr[6] = nsp + (ac + 1) * sizeof(char *);

    /* put $sp below vectors, leaving 32 byte argsave */
    Gpr[29] = nsp - 32;

    /* vectors start at nsp; strings after vectors */
    vsp = (char **) nsp;
    ssp = (char *) (nsp + vlen);

    /* build argument vector on stack */
    for (i = 0; i < ac; i++) {
	*vsp++ = ssp;
	strcpy (ssp, av[i]);
	ssp += strlen(av[i]) + 1;
    }
    *vsp++ = (char *)0;

    /* build environment vector on stack */
    if (ec)
	envbuild (vsp, ssp);
    else 
	*vsp++ = (char *)0;
}


const Optdesc         g_opts[] =
{
    {"-s", "don't set client sp"},
    {"-t", "time execution"},
    {"-e <adr>", "start address"},
    {"-b <bptadr>", "temporary breakpoint"},
    {"-- <args>", "args to be passed to client"},
    {0}};

/*************************************************************
 *  go(ac,av), the 'g' command
 */
go (ac, av)
     int             ac;
     char           *av[];
{
    word            adr;
    int             i, j, sflag;
    extern int	    optind;
    extern char    *optarg;
    int             c;

    sflag = 0;
    BptTmp.addr = NO_BPT;
    BptTrc.addr = NO_BPT;
    BptTrcb.addr = NO_BPT;

    strcpy (clientcmd, av[0]);
    strcat (clientcmd, " ");

    optind = 0;
    while ((c = getopt (ac, av, "b:e:st")) != EOF)
      switch (c) {
      case 's':
	  sflag = 1; 
	  break;
      case 't':
	  strcpy (clientcmd, "time ");
	  break;
      case 'b':
	  if (!get_rsa (&adr, optarg))
	    return (-1);
	  BptTmp.addr = adr;
	  break;
      case 'e':
	  if (!get_rsa (&adr, optarg))
	    return (-1);
	  Epc = adr;
	  break;
      default:
	  return (-1);
      }

    while (optind < ac) {
	strcat (clientcmd, av[optind++]);
	strcat (clientcmd, " ");
    }

    if (!sflag)
	Gpr[29] = clienttos ();

    clientac = argvize (clientav, clientcmd);
    initstack (clientac, clientav, 1);

    clrhndlrs ();
    closelst (2);
    Status = initial_sr;
#ifdef Icr
    /* RM7000 interrupt extensions */
    Icr = IplLo = IplHi = 0;
#endif
    sbdenable (getmachtype ()); /* set up i/u hardware */
#ifdef FLOATINGPT
    Fcr = 0;		/* clear any outstanding exceptions / enables */
#endif

    if (setjmp (&gobuf) == 0) {
	gobufvalid = 1;
	goclient (0);
    }
    gobufvalid = 0;
    swlst(1);
    return 0;
}

/*************************************************************
 *  cont(ac,av) the continue command
 */
cont (ac, av)
     int             ac;
     char           *av[];
{
    word            adr;

    BptTmp.addr = NO_BPT;
    BptTrc.addr = NO_BPT;
    BptTrcb.addr = NO_BPT;
    if (ac > 1) {
	if (!get_rsa (&adr, av[1]))
	    return (-1);
	BptTmp.addr = adr;
    }
    goclient (1);
}

const Optdesc         t_opts[] =
{
    {"-v", "verbose, list each step"},
    {"-b", "capture only branches"},
    {"-c", "capture only calls (jal)"},
    {"-i", "stop on pc invalid"},
    {"-m adr val", "stop on mem equal"},
    {"-M adr val", "stop on mem not equal"},
    {"-r reg val", "stop on reg equal"},
    {"-R reg val", "stop on reg not equal"},
    {0}};

/*************************************************************
 *  trace(ac,av) the 't' (single-step) command
 */
trace (ac, av)
     int             ac;
     char           *av[];
{
    int             stepover, multi, i, j, n;
    word            target;
    sreg_t	    *reg;
    unsigned long   adr, val;

    trace_over = 0;
    if (strequ (av[0], "to"))
	trace_over = 1;

    n = multi = trace_verbose = trace_invalid = 0;
    trace_bflag = trace_cflag = 0;
    for (i = 0; i < STOPMAX; i++)
	stopval[i].addr = 0;
    for (i = 1; i < ac; i++) {
	if (av[i][0] == '-') {
	    for (j = 1; av[i][j] != 0; j++) {
		if (av[i][j] == 'v') {
		    trace_verbose = 1;
		    trace_count = 0;
		    multi = 1;
		} else if (av[i][j] == 'm' || av[i][j] == 'M') {
		    if (i + 2 >= ac) {
			printf ("bad arg count\n");
			return (-1);
		    }
		    if (!get_rsa (&adr, av[i + 1]))
			return (-1);
		    if (!get_rsa (&val, av[i + 2]))
			return (-1);
		    if (!addstop (adr, val, "MEMORY", av[i][j]))
			return (1);
		    trace_count = 0;
		    multi = 1;
		    i += 2;
		    break;
		} else if (av[i][j] == 'r' || av[i][j] == 'R') {
		    if (i + 2 >= ac) {
			printf ("bad arg count\n");
			return (-1);
		    }
		    if (!getregadr (&reg, av[i + 1])) {
			printf ("%s: bad reg name\n", av[i + 1]);
			return (-1);
		    }
		    if (!get_rsa (&val, av[i + 2]))
			return (-1);
		    if (!addstop (*reg, val, av[i + 1], av[i][j]))
			return (1);
		    trace_count = 0;
		    multi = 1;
		    i += 2;
		    break;
		} else if (av[i][j] == 'b')
		    trace_bflag = 1;
		else if (av[i][j] == 'c')
		    trace_cflag = 1;
		else if (av[i][j] == 'i') {
		    trace_invalid = 1;
		    trace_count = 0;
		    multi = 1;
		} else {
		    printf ("%c: unrecognized option\n", av[i][j]);
		    return (-1);
		}
	    }
	} else {
	    if (n == 0) {
		if (!get_rsa (&trace_count, av[i]))
		    return (-1);
		multi = 1;
	    } else {
		printf ("%s: unrecognized argument\n", av[i]);
		return (-1);
	    }
	    n++;
	}
    }

    if (setTrcbp (Epc, trace_over))
	return (1);
    clrpchist ();
    if (multi)
	trace_mode = TRACE_TN;
    else
	trace_mode = TRACE_TB;
    store_trace_breakpoint ();
    _go ();
}

/*************************************************************
 *  addstop(adr,val,name,sense)
 */
addstop (adr, val, name, sense)
     unsigned long   adr, val;
     char           *name, sense;
{
    int             i;

    for (i = 0; i < STOPMAX; i++) {
	if (stopval[i].addr == 0)
	    break;
    }
    if (i >= STOPMAX) {
	printf ("stopval table full\n");
	return (0);
    }
    stopval[i].addr = adr;
    stopval[i].value = val;
    strcpy (stopval[i].name, name);
    if (sense == 'M' || sense == 'R')
	stopval[i].sense = 1;
    else
	stopval[i].sense = 0;
    return (1);
}


/* Remove a breakpoint */
int
clrbpt (wpt, adr, len)
    int wpt;
    long adr;
    long len;
{
    int j;

    for (j = 0; j < MAX_BPT; j++) {
	if (Bpt[j].addr == adr && Bpt[j].len == len && Bpt[j].wpt == wpt) {
	    if (wpt)
		_mips_watchpoint_clear (wpt, -1, adr, len);
	    Bpt[j].addr = NO_BPT;
	    if (Bpt[j].cmdstr)
		free (Bpt[j].cmdstr);
	    return (1);
	}
    }
    return (0);
}


#define overlap(as, ae, bs, be) ((ae) > (bs) && (be) > (as))

int
_mips_watchpoint_set_callback (int asid, vaddr_t va, size_t len)
{
    extern char _ftext[], etext[], _fdata[], edata[], _fbss[], end[];
    extern char *heaptop;
    unsigned long start, finish;

    /* check that virtual address is in KSEG0/1 */
    if (va < K0BASE || va + len > K2BASE)
	return MIPS_WP_BADADDR;

    /* now check physical addresses */
    start = K0_TO_PHYS (va);
    finish = start + len;

    if (overlap (start, finish, K0_TO_PHYS (_ftext), K0_TO_PHYS (etext)))
	/* overlaps PMON code */
	return MIPS_WP_OVERLAP;

    if (overlap (start, finish, K0_TO_PHYS (_fdata), K0_TO_PHYS (heaptop)))
	/* overlaps PMON data */
	return MIPS_WP_OVERLAP;

    if (overlap (start, finish, memorysize, memorysize + 512*1024 ))
	/* overlaps network data area */
	return MIPS_WP_OVERLAP;

    return MIPS_WP_OK;
}


/* Insert a breakpoint */
int
setbpt (wpt, adr, len)
    int wpt;
    long adr;
    long len;
{
    int j;

    /* remove any duplicates */
    clrbpt (wpt, adr, len);

    for (j = 0; j < MAX_BPT && Bpt[j].addr != NO_BPT; j++);
    if (MAX_BPT <= j)
	/* too many breakpoints */
	return (0);

    if (wpt) {
	int res;
	res = _mips_watchpoint_set (wpt, -1, adr, 0, len);
	if (res != MIPS_WP_OK) {
	    switch (res) {
	    case MIPS_WP_NONE:
		printf ("h/w watchpoints not available\n");
		break;
	    case MIPS_WP_NOTSUP:
		printf ("cannot handle this watchpoint type\n");
		break;
	    case MIPS_WP_INUSE:
		printf ("out of h/w watchpoint resources\n");
		break;
	    case MIPS_WP_OVERLAP:
		printf ("watchpoint overlaps PMON memory\n");
		break;
	    case MIPS_WP_BADADDR:
		printf ("unsupported watchpoint virtual address\n");
		break;
	    default:
		printf ("internal watchpoint problem\n");
		break;
	    }
	    return (0);
	}
    }

    Bpt[j].addr = adr;
    Bpt[j].len  = len;
    Bpt[j].wpt  = wpt;
    return (1);
}


const Optdesc         b_opts[] =
{
    {"-d", "hw bpt for data access"},
    {"-r", "hw bpt for data read only"},
    {"-w", "hw bpt for data write only"},
    {"-x", "hw bpt for instruction fetch"},
    {"-l", "hw bpt length"},
    {"-s", "command string"},
    {0}};

/*************************************************************
 *  setbp(ac,av) the 'b' (set breakpoint) command
 */
setbp (ac, av)
     int             ac;
     char           *av[];
{
    word            adr, i, j, wpt;
    char           *str;
    int             flag = 0;
    uword	    len = 4;

    if (ac == 1) {
	dspbpts ();
	return (0);
    }
    wpt = 0;
    str = 0;
    for (i = 1; i < ac; i++) {
	if (av[i][0] == '-') {
	    for (j = 1; av[i][j] != 0; j++) {
		if (av[i][j] == 's') {
		    i++;
		    if (i >= ac) {
			printf ("bad arg count\n");
			return (-1);
		    }
		    str = av[i];
		    break;
		}
		else if (av[i][j] == 'l') {
		    i++;
		    if (i >= ac) {
			printf ("bad arg count\n");
			return (-1);
		    }
		    if (!get_rsa (&len, av[i]))
			return (-1);
		    break;
		}
		else if (av[i][j] == 'i' || av[i][j] == 'x')
		    wpt |= MIPS_WATCHPOINT_X;
		else if (av[i][j] == 'd' || av[i][j] == 'a')
		    wpt |= MIPS_WATCHPOINT_R | MIPS_WATCHPOINT_W;
		else if (av[i][j] == 'w')
		    wpt |= MIPS_WATCHPOINT_W;
		else if (av[i][j] == 'r')
		    wpt |= MIPS_WATCHPOINT_R;
		else {
		    printf ("%c: unrecognized option\n", av[i][j]);
		    return (-1);
		}
	    }
	} else {
	    flag = 1;
	    if (!get_rsa (&adr, av[i]))
		return (-1);

	    /* remove any duplicates */
	    clrbpt (wpt, adr, len);

	    for (j = 0; j < MAX_BPT && Bpt[j].addr != NO_BPT; j++);
	    if (MAX_BPT <= j) {
		printf ("too many breakpoints\n");
		return (1);
	    }

	    if (wpt) {
		int res;
		res = _mips_watchpoint_set (wpt, -1, adr, 0, len);
		if (res != MIPS_WP_OK) {
		    switch (res) {
		    case MIPS_WP_NONE:
			printf ("h/w watchpoints not available\n");
			break;
		    case MIPS_WP_NOTSUP:
			printf ("cannot handle this watchpoint type\n");
			break;
		    case MIPS_WP_INUSE:
			printf ("out of h/w watchpoint resources\n");
			break;
		    case MIPS_WP_OVERLAP:
			printf ("watchpoint overlaps PMON memory\n");
			break;
		    case MIPS_WP_BADADDR:
			printf ("unsupported watchpoint virtual address\n");
			break;
		    default:
			printf ("internal watchpoint problem\n");
			break;
		    }
		    return (1);
		}
	    } 

	    Bpt[j].addr = adr;
	    Bpt[j].len  = len;
	    Bpt[j].wpt  = wpt;

	    printf ("Bpt %2d = %08x", j, adr);
	    printf (" %c%c%c%c len %x", 
		    (wpt == 0) ? 'b' : '-',
		    (wpt & MIPS_WATCHPOINT_R) ? 'r' : '-',
		    (wpt & MIPS_WATCHPOINT_W) ? 'w' : '-',
		    (wpt & MIPS_WATCHPOINT_X) ? 'x' : '-', len);
	    if (wpt == 0 && ((adr & 3L) != 0 || len != 4))
		printf (" -> ??");
	    if (str != 0) {
		Bpt[j].cmdstr = malloc (strlen (str) + 1);
		strcpy (Bpt[j].cmdstr, str);
		str = 0;
		printf (" \"%s\"", Bpt[j].cmdstr);
	    } else
		Bpt[j].cmdstr = 0;
	    putchar ('\n');

	    wpt = 0, len = 4;
	}
    }
    if (!flag)
	printf ("break address not specified\n");
    return (0);
}

/*************************************************************
 *  dspbpts() display all breakpoints
 */
dspbpts ()
{
    int             i, ln, siz;
    char            tmp[64], buf[100];

    siz = moresz;
    ioctl (STDIN, CBREAK, NULL);
    ln = siz;
    for (i = 0; i < MAX_BPT; i++)
	if (Bpt[i].addr != NO_BPT) {
	    sprintf (buf, "Bpt %2d = %08x ", i, Bpt[i].addr);
	    if (adr2symoff (tmp, Bpt[i].addr, 0))
		strcat (buf, tmp);

	    sprintf (tmp, "%c%c%c%c len %x", 
		     (Bpt[i].wpt == 0) ? 'b' : '-',
		     (Bpt[i].wpt & MIPS_WATCHPOINT_R) ? 'r' : '-',
		     (Bpt[i].wpt & MIPS_WATCHPOINT_W) ? 'w' : '-',
		     (Bpt[i].wpt & MIPS_WATCHPOINT_X) ? 'x' : '-',
		     Bpt[i].len);
	    strcat (buf, tmp);

	    if (Bpt[i].wpt == 0 && ((Bpt[i].addr & 3L) || Bpt[i].len != 4))
		strcat (buf, " -> ??");

	    if (Bpt[i].cmdstr) {
		sprintf (tmp, " \"%s\"", Bpt[i].cmdstr);
		strcat (buf, tmp);
	    }
	    if (more (buf, &ln, siz))
		break;
	}
}


/*************************************************************
 *  clrbp(ac,av)
 *      The 'db' command
 */
clrbp (ac, av)
     int             ac;
     char           *av[];
{
    word            adr, i, j;

    if (ac > 1) {
	for (i = j = 0; j < ac - 1; j++) {
	    if (strequ (av[1 + j], "*")) {
		clrbpts ();
		continue;
	    }
	    if (!atob (&i, av[1 + j], 10)) {
		printf ("%s: decimal number expected\n", av[1 + j]);
		return (-1);
	    }
	    if (i < MAX_BPT) {
		if (Bpt[i].addr == NO_BPT)
		    printf ("Bpt %2d not set\n", i);
		else {
		    if (Bpt[i].wpt) {
			_mips_watchpoint_clear (Bpt[i].wpt, -1, 
						Bpt[i].addr, Bpt[i].len);
			Bpt[i].wpt = 0;
		    }
		    Bpt[i].addr = NO_BPT;
		    if (Bpt[i].cmdstr)
			free (Bpt[i].cmdstr);
		}
	    }
	    else
		printf ("%d: breakpoint number too large\n", i);
	}
    } else
	dspbpts ();
    return (0);
}

/*************************************************************
 *  rm_bpts()
 */
rm_bpts ()
{
    int             i;

    if (BptTmp.addr != NO_BPT && load_word (BptTmp.addr) == BPT_CODE)
	store_word (BptTmp.addr, BptTmp.value);
    BptTmp.addr = NO_BPT;

    for (i = 0; i < MAX_BPT; i++) {
	if (Bpt[i].addr != NO_BPT && Bpt[i].wpt == 0
	    && load_word (Bpt[i].addr) == BPT_CODE)
	    store_word (Bpt[i].addr, Bpt[i].value);
    }
    remove_trace_breakpoint ();
}

/*************************************************************
 *  clrbpts()
 */
clrbpts ()
{
    int             i;

    for (i = 0; i < MAX_BPT; i++) {
	if (Bpt[i].addr != NO_BPT && Bpt[i].wpt != 0) 
	    _mips_watchpoint_clear (Bpt[i].wpt, -1, 
				    Bpt[i].addr, Bpt[i].len);
	Bpt[i].addr = NO_BPT;
	Bpt[i].wpt = 0;
	if (Bpt[i].cmdstr)
	    free (Bpt[i].cmdstr);
    }
}

/*************************************************************
 *  goclient()
 */
goclient (cont)
    int cont;
{
    if (cont || is_break_point (Epc)) {
	/* when continuing unconditionally start with a single-step, 
	   in case we were stopped by a watchpoint */
	if (setTrcbp (Epc, 0))
	    return (1);
	trace_mode = TRACE_TG;
	store_trace_breakpoint ();
    }
    else {
	trace_mode = TRACE_GB;
	store_breakpoint ();
    }
    _go ();
}


/*************************************************************
 *  godebug()
 */
godebug ()
{
    /* remote debugger has responsibility for issuing a single-step
       and reinserting breakpoints before continuing, but we must
       insert any breakpoints or watchpoints that it has set up
       using the 'B' request. */
    trace_mode = TRACE_DC;
    store_breakpoint ();
    _go ();
}

/*************************************************************
 * sstep()
 */
sstep ()
{

    if (setTrcbp (Epc, 0))
	return (1);
    trace_mode = TRACE_DS;
    store_trace_breakpoint ();
    _go ();
}


reboot ()
{
    struct termio tio;
    extern int     *curlst;

    if (curlst) {
	printf ("\r\n");
	memset (&tio, 0, sizeof (tio));
	ioctl (0, TCSETAW, &tio);
    }

#if 0
    start ();
#else
    {
	void (*rebootaddr)() = (void (*))0xbfc00000;
	(*rebootaddr)();
    }
#endif
}


/*************************************************************
 *  pmexception(epc,cause)
 *      An (fatal) exception has been generated within PMON
 */
pmexception (epc, cause, ra, badva)
    unsigned int epc, cause, ra, badva;
{
    extern char    *sbdexception();
    char 	   *exc;
    extern int     *curlst;
    int 	   i;

#ifdef INET
    /* lock out all "interrupts" */
    (void) splhigh();
#endif

    /* display exception on alpha-display before attempting to print */
    exc = 0;
    for (i = (curlst ? 1 : 5); i != 0; i--)
      exc = sbdexception (epc, cause, ra, badva, exc);

#if 0
    if (!curlst)
	reboot ();
#endif

    if (exc)
      printf ("\r\n%s\r\n", exc);
#ifdef R4000
    else if (cause == CEXC_CACHE) {
	printf ("\r\nPMON Uncorrectable Cache Error\r\n");
	printf ("ErrPC:    %08x\r\n", epc);
	printf ("CacheErr: %08x\r\n", badva);
	printf ("ECC:      %08x\r\n", get_ecc());
	reboot ();
    }
#endif
    else 
      printf ("\r\nPMON %s exception\r\n", getexcname (cause & CAUSE_EXCMASK));

    printf ("EPC:      %08x\r\n", epc);
    printf ("RA:       %08x\r\n", ra);
    printf ("Cause:    %08x\r\n", cause);
    switch (cause & CAUSE_EXCMASK) {
    case CEXC_MOD:
    case CEXC_TLBL:
    case CEXC_TLBS:
    case CEXC_ADEL:
    case CEXC_ADES:
	printf ("BadVaddr: %08x\r\n", badva);
    }

#ifdef INET
    /* XXX something is going wrong when we try to reset the sonic interface */
    /* XXX for now we'll just completely reinitialise from scratch */
    /*reset_net ();*/
    reboot ();
#else
#if 1
    if (!curlst)
	reboot ();
#endif
    closelst (0);
    curlst = 0;
    main ();
#endif
}


/*************************************************************
 *  exception()
 *      An exception has been generated within the client
 */
exception ()
{
    extern char    *sbddbgintr(unsigned int);
    int             i, j, flag;
    char            tmp[80], *p = 0;
    int		    exc;
    int		    hit = 0; 
    vaddr_t	    hitaddr;
    size_t	    hitlen;

    exc = Cause & CAUSE_EXCMASK;
    if (exc == CEXC_INT && (p = sbddbgintr (Cause & Status)))
      printf ("\r\n%s Interrupt\r\n", p);

    /* remove watchpoints */
    _mips_watchpoint_remove ();

    /* check for watchpoint hit */
    hit = _mips_watchpoint_hit (&hitaddr, &hitlen);

    if ((trace_mode == TRACE_DC || trace_mode == TRACE_GB)
	&& (hit & MIPS_WATCHPOINT_INEXACT)) {
	/* step over inexact watchpoint */
#if 0
	printf ("skipping inexact watchpoint @ %lx: a=%lx l=%lx\n",
		(long)Epc, hitaddr, hitlen);
#endif
	if (setTrcbp (Epc, 0) == 0) {
	    trace_mode = (trace_mode == TRACE_DC) ? TRACE_DW : TRACE_TW;
	    store_trace_breakpoint ();
	    _go ();
	}
    }

    if (trace_mode == TRACE_DW || trace_mode == TRACE_TW) {
	/* restart after inexact watchpoint step */
#if 0
	printf ("restart after inexact watchpoint @ %lx\n", (long)Epc);
#endif
	trace_mode = (trace_mode == TRACE_DW) ? TRACE_DC : TRACE_GB;
	remove_trace_breakpoint ();
	if (exc == CEXC_BP && !is_break_point (Epc)) {
	    _mips_watchpoint_insert ();
	    _go ();
	}
    }

#if 0
    if (hit)
	printf ("hit watchpoint @ %lx: a=%lx l=%lx\n",
		(long)Epc, hitaddr, hitlen);
#endif

    if (trace_mode == TRACE_DC || trace_mode == TRACE_DS) {
	/* pass on remote debug exceptions */
	rm_bpts ();
	dbgmode ();
    }

    if (exc != CEXC_BP && hit == 0)
    {
	if (!p)
	  printf ("\r\nException Cause=%s (%08x)\r\n",
		  getexcname (Cause & CAUSE_EXCMASK), Cause);
	stop (0);
    } else if (trace_mode == TRACE_NO) {	/* no bpts set */
	if (hit)
	    printf ("\r\nUnexpected H/w Breakpoint (addr %x)\r\n", 
		    hitaddr);
	else
	    printf ("\r\nUnexpected Breakpoint\r\n");
	stop (0);
    } else if (trace_mode == TRACE_GB) {	/* go & break */
	if (hit) {
	    vaddr_t hita;
	    hita = (hit & MIPS_WATCHPOINT_VADDR) ? hitaddr
		: KVA_TO_PA (hitaddr);
	    for (i = 0; i < MAX_BPT; i++) {
		if (Bpt[i].wpt & hit) {
		    vaddr_t bpa;
		    bpa =  Bpt[i].addr;
		    if (!(hit & MIPS_WATCHPOINT_VADDR))
			bpa = KVA_TO_PA (bpa);
		    if (bpa < hita + hitlen && hita < bpa + Bpt[i].len) {
			printf ("\r\nStopped at H/w Bpt %d (addr %x)\r\n", 
				i, hitaddr);
			break;
		    }
		}
	    }
	    if (i >= MAX_BPT)
		printf ("\r\nStopped by unknown H/w Bpt (addr %x)\n", hitaddr);
	}
	else {
	    for (i = 0; i < MAX_BPT; i++) {
		if (Bpt[i].addr == Epc && Bpt[i].wpt == 0) {
		    printf ("\r\nStopped at Bpt %d\n", i);
		    break;
		}
	    }
	}
	rm_bpts ();
	stop (i < MAX_BPT ? Bpt[i].cmdstr : 0);
    }
    
    remove_trace_breakpoint ();
    
    if (trace_mode == TRACE_TB)
	stop (0);		/* trace & break */
    else if (trace_mode == TRACE_TN) {
	for (i = 0; i < MAX_BPT; i++) {
	    if (Bpt[i].addr == Epc && Bpt[i].wpt == 0) {
		printf ("\r\nStopped at Bpt %d\r\n", i);
		stop (Bpt[i].cmdstr);
	    }
	}
	if (hit) {
	    /* XXX how could we handle this better */
	    printf ("\r\nStopped by %sH/w Bpt\r\n", 
		    hit & MIPS_WATCHPOINT_INEXACT ? "Inexact " : "");
	    stop (0);
	}
	if (trace_invalid && !is_validpc (Epc)) {
	    printf ("\r\nStopped: Invalid PC value\r\n");
	    stop (0);
	}
	for (i = 0; i < STOPMAX; i++) {
	    if (stopval[i].addr == 0)
		continue;
	    if ((stopval[i].sense == 0 &&
		 load_word (stopval[i].addr) == stopval[i].value)
		|| (stopval[i].sense == 1 &&
		    load_word (stopval[i].addr) != stopval[i].value)) {
		if (stopval[i].sense == 0)
		    p = " == ";
		else
		    p = " != ";
		if (strequ (stopval[i].name, "MEMORY"))
		    printf ("\r\nStopped: 0x%08x%s0x%08x\r\n",
			    stopval[i].addr, p, stopval[i].value);
		else
		    printf ("\r\nStopped: %s%s0x%08x\r\n", stopval[i].name,
			    p, stopval[i].value);
		stop (0);
	    }
	}
	flag = 1;
	if (trace_bflag || trace_cflag) {
	    if (trace_bflag && is_branch (Epc))
		flag = 1;
	    else if (trace_cflag && is_jal (Epc))
		flag = 1;
	    else
		flag = 0;
	}
	if (flag) {
	    addpchist (Epc);
	    if (trace_verbose) {
		disasm (tmp, Epc, load_word ((word) Epc));
		printf ("%s\r\n", tmp);
		if (is_branch (Epc)) {
		    /* print the branch delay slot too */
		    disasm (tmp, Epc + 4, load_word ((word) Epc + 4));
		    mkcomment (tmp, "# bdslot", 10);
		    printf ("%s\r\n", tmp);
		}
	    } else
		dotik (256, 1);
	} else
	    dotik (256, 1);
	if (trace_count)
	    trace_count--;
	if (trace_count == 1)
	    trace_mode = TRACE_TB;
	if (setTrcbp (Epc, trace_over))
	    stop (0);
	store_trace_breakpoint ();
	_go ();
    }

/* else TRACE_TG | TRACE_DG  trace & go, set on g or c if starting at bpt */
    trace_mode = (trace_mode == TRACE_DG) ? TRACE_DC : TRACE_GB;
    store_breakpoint ();
    _go ();
}

/*************************************************************
 *  stop(cmdstr)
 */
stop (cmdstr)
     char           *cmdstr;
{
    char            cmd[LINESZ];

    swlst (1);
    trace_mode = TRACE_NO;
    gobufvalid = 0;
    if (cmdstr)
	strcpy (cmd, cmdstr);
    else
	strcpy (cmd, getenv ("brkcmd"));
    do_cmd (cmd);
    main ();
}

/*************************************************************
 *  store_breakpoint()
 */
store_breakpoint ()
{
    int             i;

    for (i = 0; i < MAX_BPT; i++) {
	if (BptTmp.addr == Bpt[i].addr)
	    BptTmp.addr = NO_BPT;
	if (BptTrc.addr == Bpt[i].addr)
	    BptTrc.addr = NO_BPT;
	if (BptTrcb.addr == Bpt[i].addr)
	    BptTrcb.addr = NO_BPT;
    }
    if (BptTrc.addr == BptTmp.addr)
	BptTrc.addr = NO_BPT;
    if (BptTrcb.addr == BptTmp.addr || BptTrcb.addr == BptTrc.addr)
	BptTrcb.addr = NO_BPT;

    for (i = 0; i < MAX_BPT; i++)
	if (Bpt[i].addr != NO_BPT && Bpt[i].wpt == 0) {
	    Bpt[i].value = load_word (Bpt[i].addr);
	    store_word (Bpt[i].addr, BPT_CODE);
	}
    if (BptTmp.addr != NO_BPT) {
	BptTmp.value = load_word (BptTmp.addr);
	store_word (BptTmp.addr, BPT_CODE);
    }
    store_trace_breakpoint ();
    _mips_watchpoint_insert ();
}

/*************************************************************
 *  store_trace_breakpoint()
 */
store_trace_breakpoint ()
{
    if (BptTrc.addr != NO_BPT) {
	BptTrc.value = load_word (BptTrc.addr);
	store_word (BptTrc.addr, BPT_CODE);
    }
    if (BptTrcb.addr != NO_BPT) {
	BptTrcb.value = load_word (BptTrcb.addr);
	store_word (BptTrcb.addr, BPT_CODE);
    }
}

remove_trace_breakpoint ()
{
    if (BptTrc.addr != NO_BPT && load_word (BptTrc.addr) == BPT_CODE)
      store_word (BptTrc.addr, BptTrc.value);
    BptTrc.addr = NO_BPT;
    if (BptTrcb.addr != NO_BPT && load_word (BptTrcb.addr) == BPT_CODE)
      store_word (BptTrcb.addr, BptTrcb.value);
    BptTrcb.addr = NO_BPT;
}

/*************************************************************
 *  int is_break_point(adr)
 */
static int 
is_break_point (adr)
     word            adr;
{
    int             i;

    for (i = 0; i < MAX_BPT; i++)
	if (Bpt[i].addr == adr && Bpt[i].wpt == 0)
	    return (1);
    if (BptTmp.addr == adr)
	return (1);
    return (0);
}

#define NVALIDPC	10
static unsigned long	validpc[NVALIDPC];
static int		nvalidpc = -1;


flush_validpc ()
{
    nvalidpc = -1;
}


/* chg_validpc: called if variable is changed */
chg_validpc (name, value)
    char *name, *value;
{
    char           *av[NVALIDPC], tmp[80];

    strcpy (tmp, value);
    if (argvize (av, tmp) % 2 != 0) {
	printf ("validpc variable must have even number of values\n");
	return (0);
    }
    /* don't check the values here, symbols may not be loaded */
    flush_validpc ();
    return (1);
}


static void
compute_validpc ()
{
    char           *av[NVALIDPC], tmp[80];
    int		    ac, i;

    strcpy (tmp, getenv ("validpc"));
    ac = argvize (av, tmp);
    nvalidpc = 0;
    
    for (i = 0; i < ac; i += 2) {
	if (!get_rsa (&validpc[nvalidpc], av[i]))
	  continue;
	if (!get_rsa (&validpc[nvalidpc+1], av[i+1]))
	  continue;
	nvalidpc += 2;
    }
}


/*************************************************************
 *  is_validpc(adr)
 */
static int
is_validpc (adr)
    word adr;
{
    int         i;

    if (adr & 3 || (uword)adr < K0BASE || (uword)adr >= K2BASE)
      return (0);

    if (nvalidpc < 0)
      compute_validpc ();

    for (i = 0; i < nvalidpc; i += 2)
      if ((uword)adr >= validpc[i] && (uword)adr < validpc[i+1])
	return (1);

    return (0);
}


/*************************************************************
 *  addpchist(adr)
 */
static void
addpchist (adr)
    word   adr;
{
    pchist_d[pchist_ip] = adr;
    pchist_ip = incmod (pchist_ip, PCHISTSZ);
    if (pchist_ip == pchist_op)
	pchist_op = incmod (pchist_op, PCHISTSZ);
}

/*************************************************************
 *  clrpchist()
 */
clrpchist ()
{
    pchist_ip = pchist_op = 0;
}

/*************************************************************
 *  unsigned long getpchist(n)
 */
unsigned long 
getpchist (n)
     int             n;
{
    int             i;

    i = pchist_ip - n - 1;
    if (i < 0)
	i += PCHISTSZ + 1;
    if (incmod (i, PCHISTSZ) == pchist_op)
	return (0);
    return (pchist_d[i]);
}

/*************************************************************
 *  setTrcbp(adr,stepover)
 */
static int
setTrcbp (adr, stepover)
     word	     adr;
     int             stepover;
{
    unsigned long   target;

    BptTrc.addr = NO_BPT;
    BptTrcb.addr = NO_BPT;
    if (is_branch (adr)) {
#if 0
	if (is_branch (adr + 4)) {
	    printf ("branch in delay slot\n");
	    return (1);
	}
#endif
	target = branch_target_address (adr);
	if (target == adr)
	    target = adr + 8;	/* skip self branches */
	if (is_conditional_branch (adr) && target != adr + 8)
	    BptTrc.addr = adr + 8;
	if (is_jal (adr) && stepover)
	    BptTrc.addr = adr + 8;
	else if (is_jr (adr) && !is_writeable (target))
	    BptTrc.addr = Gpr[31];
	else
	    BptTrcb.addr = target;
    } else
	BptTrc.addr = adr + 4;
    return (0);
}

#include "mips_opcode.h"

#define MIPS_JR_RA	0x03e00008	/* instruction code for jr ra */
#define MAX_PROLOGUE	32		/* max insns in prologue */


static struct savedreg {
    void	*addr;
    short	base;
    short	size;
    sreg_t	val;
} regs[32];


static int
getsavedreg (int rn, sreg_t *rv)
{
    int saved = 0;

    if (regs[rn].addr) {
	if (regs[rn].size == 8)
	  regs[rn].val = load_dword (regs[rn].addr);
	else
	  regs[rn].val = load_word (regs[rn].addr);
	regs[rn].addr = 0;
	saved = 1;
    }
    *rv = regs[rn].val;
    return saved;
}



static void
setregsave (int rn, int base, void *addr, int size)
{
    regs[rn].addr = addr;
    regs[rn].base = base;
    regs[rn].size = size;
}


static void
adjregsaves (int obase, int nbase, int adjust)
{
    int r;

    for (r = 0; r < 32; r++) {
	if (regs[r].addr && regs[r].base == obase) {
	    regs[r].addr += adjust;
	    regs[r].base = nbase;
	}
    }
}


/*
 * Print a stack backtrace.
 */
int
stacktrace(ac, av)
    int ac;
    char **av;
{
    uword pc, sp, fp, ra, va, subr;
    long base;
    sreg_t rv;
    unsigned long mask;
    InstFmt i;
    int cont, stksize, adjust, r;
    uword cnt, siz;
    char *p;
#ifdef R4000
    int regsize = matchenv ("regsize");
#else
#define regsize REGSZ_32
#endif
    int vflag = 0;
    extern int optind;
    extern char *optarg;
    int c, rn;

    optind = 0;
    while ((c = getopt (ac, av, "v")) != EOF)
      switch (c) {
      case 'v':
	  vflag++;
	  break;
      default:
	  return (-1);
      }

    cnt = siz = moresz;
    if (optind < ac) {
	if (!get_rsa (&cnt, av[optind++]))
	  return (-1);
	siz = 0;
    }

    if (optind != ac)
      return (-1);
    
    ioctl (STDIN, CBREAK, NULL);

    bzero (regs, sizeof (regs));
    sp = Gpr[R_SP];
    fp = Gpr[R_FP];
    ra = Gpr[R_RA];
    pc = Epc;

 stkloop:
    stksize = adjust = 0;

    if ((sp & 3) || sp < K0BASE || sp >= K2BASE) {
	printf("invalid SP=0x%x\n", sp);
	ra = subr = 0;
	goto stkdone;
    }

    /*
     * Find the beginning of the current subroutine by scanning backwards
     * from the current PC for the end of the previous subroutine.
     */
    va = pc;
    do {
	va -= sizeof(int);
	if (!is_validpc (va)) {
	    printf("invalid PC=0x%x\n", va);
	    ra = subr = 0;
	    goto stkdone;
	}
    } while (load_word(va) != MIPS_JR_RA);

    va += 2 * sizeof(int);	/* skip back over "jr ra" & delay slot */

    /* skip over nulls which might separate .o files */
    while (load_word(va) == 0 && va < pc)
      va += sizeof(int);
    subr = va;

    /* scan forwards to find stack size and any saved registers */
    mask = 0;

    cont = (pc - va) / sizeof(int);
    for ( ; cont > 0 ; va += sizeof(int), cont--) {

	i.word = load_word(va);
	switch (i.JType.op) {
	case OP_SPECIAL:
	    switch (i.RType.func) {
	    case OP_JR:
	    case OP_JALR:
		if (cont > 2)
		    cont = 2; /* stop after next instruction */
		break;
		
	    case OP_SYSCALL:
	    case OP_BREAK:
		cont = 1; /* stop now */
		break;

	    case OP_OR:
	    case OP_DADDU:
	    case OP_ADDU:
		/* one of RS or RT must be $0, for this to be a MOVE */
		if (i.RType.rt == 0)
		    rn = i.RType.rs;
		else if (i.RType.rs == 0) 
		    rn = i.RType.rt;
		else
		    break;
		
		if (i.RType.rd == 30 && rn == 29) {
		    /* move $fp,$sp */
		    /* check for use of alloca ($sp != $fp) */
		    if (fp > sp) {
			/* base previous register-saves on more reliable $fp */
			adjregsaves (29, 30, adjust = fp - sp);
			/* get the correct value of $sp at this point */
			sp += adjust;
		    }
		}
		break;
	    }
	    break;

	case OP_REGIMM:
	    switch (i.IType.rt) {
	    case OP_BLTZ:
	    case OP_BGEZ:
	    case OP_BLTZL:
	    case OP_BGEZL:
	    case OP_BLTZAL:
	    case OP_BGEZAL:
	    case OP_BLTZALL:
	    case OP_BGEZALL:
		if (cont > 2)
		    cont = 2;		/* stop after next instruction */
		break;
		
	    case OP_TGEI:
	    case OP_TGEIU:
	    case OP_TLTI:
	    case OP_TLTIU:
	    case OP_TEQI:
	    case OP_TNEI:
		cont = 1;		/* stop now */
		break;
	    }
	    break;

	case OP_COP0:
	case OP_COP1:
	case OP_COP2:
	case OP_COP3:
	    switch (i.RType.rs) {
	    case OP_BCx:
	    case OP_BCy:
		if (cont > 2)
		    cont = 2;		/* stop after next instruction */
	    };
	    break;

	case OP_SW:
	case OP_SD:
	    /* look for saved registers and store a pointer to them */
	    if (i.IType.rs == 29)
	      base = sp;
	    else if (i.IType.rs == 30)
	      base = fp;
	    else
	      break;
	    if (!(mask & (1 << i.IType.rt))) {
		mask |= 1 << i.IType.rt;
		setregsave (i.IType.rt, i.IType.rs,
			    (void *)base + (short)i.IType.imm,
			    (i.JType.op == OP_SW) ? 4 : 8);
	    }
	    break;
	    
	case OP_ADDIU:
	case OP_DADDIU:
	    /* stack or frame pointer adjustment? */
	    if (i.IType.rs == 29) {
		if (i.IType.rt == 29) {		/* addu $sp,size */
		    /* adjust previous saves, which should be based on old val */
		    adjregsaves (29, 29, -(short)i.IType.imm);
		    /* but we assume that current sp is now correct */
		    stksize += -(short)i.IType.imm;
		} else if (i.IType.rt == 30) {	/* addu $fp,$sp,size */
		    /* check for use of alloca ($sp + size != $fp) */
		    if (fp > sp + (short)i.IType.imm) {
			/* base previous register-saves on more reliable $fp */
			adjust = fp - (sp + (short)i.IType.imm);
			adjregsaves (29, 30, adjust);
			/* we also get the correct value of $sp at this point */
			sp += adjust;
		    }
		}
	    }
	    break;
	}
    }

    /* fetch saved fp and ra registers from stack */
    if (getsavedreg (30, &rv))
      fp = rv;
    if (getsavedreg (31, &rv))
      ra = rv;

 stkdone:
    p = prnbuf;
    if (!adr2symoff (p, pc, 24)) {
	if (subr) 
	  sprintf (p, "          0x%08x+0x%-4x", subr, pc - subr);
	else
	  sprintf (p, "                 0x%08x", pc);

    }
    p += strlen(p);

    /* print out argument reg values, if they were saved */
    *p++ = '(';
    for (r = R_A0; r <= R_A3; r++) {
	if (!getsavedreg (r, &rv))
	  /* reg not saved (optimised code or unused arg) */
	  break;
	if (r != R_A0)
	  *p++ = ',';
	if (regsize == REGSZ_64)
	  sprintf (p, "0x%llx", rv);
	else
	  sprintf (p, "0x%lx", (word)rv);
	p += strlen(p);
    }
    *p++ = ')'; *p++ = '\0';
    if (more (prnbuf, &cnt, siz))
      return (0);

    if (vflag) {
	p = prnbuf;
	strcpy (p, "            from "); 
	p += strlen (p);;
	if (!adr2symoff (p, ra, 12))
	  sprintf (p, "         0x%08x", ra);
	p += strlen(p);
	sprintf (p, " frame=0x%08x size=%-4d", sp, stksize);
	if (adjust) {
	    p += strlen(p);
	    sprintf (p, " (alloca=%-4d)", adjust);
	}
	if (more (prnbuf, &cnt, siz))
	  return (0);
    }

    if (ra) {
	if (pc == ra && stksize == 0)
	  more ("stacktrace loop!", &cnt, siz);
	else {
	    pc = ra; ra = 0;
	    sp += stksize;
	    goto stkloop;
	}
    }

    return 0;
}
