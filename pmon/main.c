/* $Id: main.c,v 1.26 2001/09/04 17:40:06 chris Exp $ */
#include "mips.h"
#include "pmon.h"
#include "stdio.h"
#include "termio.h"
#include "string.h"
#include "signal.h"

#ifdef MIPSEB
char            endian[] = "EB";
#else
char            endian[] = "EL";
#endif

Bps             Bpt[MAX_AC];	/* user break points                    */
Bps             BptTmp;		/* tmp bpt used for continue cmd        */
Bps             BptTrc;		/* bpt for tracing                      */
Bps             BptTrcb;	/* bpt for tracing through branches     */

reg_t           DBGREG[NREGS];	/* debugger's register value holder */

int             machtype;
jmp_buf         jmpb;		/* non-local goto jump buffer */
char            line[LINESZ + 1];	/* input line */
int             pmlst, clilst;	/* list of files opened by: PMON & client */
extern int     *curlst;		/* list of files opened by: current context */
struct termio	clitbuf;	/* client terminal mode */
unsigned long   initial_sr;
int             memorysize;
char            prnbuf[LINESZ + 8];	/* commonly used print buffer */

int 		_pmon_in_ram;
int             repeating_cmd;

#if defined(FLOATINGPT) && !defined(NEWFP)
extern struct c1state *c1dat;
#endif

const Optdesc         sh_opts[] =
{
    {"^P", "recall previous cmd"},
    {"^N", "recall next cmd"},
    {"^F", "move cursor right"},
    {"^B", "move cursor left"},
    {"^A", "move cursor far left"},
    {"^E", "move cursor far right"},
    {"^D", "delete char at cursor"},
    {"^H", "delete char before cursor"},
    {"^K", "delete line from cursor"},
    {"", ""},
    {"!!", "repeat last cmd"},
    {"!str", "recall and execute cmd str"},
    {"!num", "recall and execute cmd num"},
    {"", ""},
    {"+-*/()", "operators"},
    {"^addr", "contents of address"},
    {"@name", "contents of register"},
    {"&name", "value of symbol"},
    {"0xnum", "hex number"},
    {"0onum", "octal number"},
    {"0tnum", "decimal number"},
    {0}};


#ifdef INET
static void
pmon_intr (int dummy)
{
    sigsetmask (0);
    longjmp (jmpb, 1);
}
#endif


/*************************************************************
 *  main()
 */
main ()
{
    int             i, t1;
    char            prompt[20], tmp[8], *p;

    if (setjmp (jmpb))
	printf (" break!\r\n");
#ifdef INET
    signal (SIGINT, pmon_intr);
#else
    ioctl (STDIN, SETINTR, jmpb);
#endif

    rm_bpts ();
    set_sr (initial_sr);

    while (1) {
	closelst (0);
	swlst (1);
	ioctl (STDIN, SETSANE, NULL);
	if (_pmon_in_ram)
	    strcpy (prompt, "PRAM> ");
	else
	    strcpy (prompt, getenv ("prompt"));
	if (p = strchr (prompt, '!')) {
	    strdchr (p);	/* delete the bang */
	    sprintf (tmp, "%d", histno);
	    stristr (p, tmp);
	}
	printf ("%s", prompt);
	get_cmd (line);
	do_cmd (line);
    }
}

void
autorun (char *s)
{
    char buf[LINESZ];
    char *d;
    unsigned long delay, t, lastt;
    unsigned int cnt;
    struct termio sav;
    int i;

    if (s && strlen(s)) {
	d = getenv ("bootdelay");
	if (!d || !atob (&delay, d, 10) || delay < 0 || delay > 99)
	    delay = 20;

	SBD_DISPLAY ("AUTO", CHKPNT_AUTO);
	printf ("Autoboot command: \"%.55s\"\nPress any key to cancel.\n", s);
	ioctl (STDIN, CBREAK, &sav);
	lastt = 0;
	delay++;
	do {
#if defined(RTC) || defined(INET)
	    t = __time (0);
	    if (t != lastt) {
		printf ("\r%2d", --delay);
		lastt = t;
	    }
#else
printf ("starting delay\n");
	    for (i = 10000000; i != 0; i--) continue;
printf ("delay...\n");
	    printf ("\r%2d", --delay);
#endif
	    ioctl (STDIN, FIONREAD, &cnt);
	} while (delay != 0 && cnt == 0);
	ioctl (STDIN, TCSETAF, &sav);
	putchar ('\n');
	if (cnt == 0) {
	    strcpy (buf, s);
	    do_cmd (buf);
	}
    }
}


#pragma weak sbdmachinfo
void sbdmachinfo()
{
}


/*************************************************************
 *  dbginit(adr)
 */
dbginit (adr)
     char           *adr;
{
    static const int fraction[] = {1000000, 100000, 10000, 1000, 100, 10, 1};
    int             i, memsize, freq, frac;
    char	    *failure = 0;
    char	    *s;
    ureg_t	    v;
#ifdef INET
    int		    neterr = 0;
#endif

    memsize = memorysize;
#if defined(LOCAL_MEM) && (LOCAL_MEM!=0)
    memsize -= LOCAL_MEM;
#endif

    SBD_DISPLAY ("ENVI", CHKPNT_ENVI);
    envinit ();

    SBD_DISPLAY ("SBDD", CHKPNT_SBDD);
    sbddevinit ();

    SBD_DISPLAY ("DEVI", CHKPNT_DEVI);
    devinit ();

    failure = getenv ("itfailure");

#ifdef INET
    if (failure) {
	s = getenv ("itfailcodes");
	if (s) {
	    /* check for an ethernet test error */
	    char buf[128];
	    strncpy (buf, s, sizeof(buf)-1);
	    for (s = strtok (buf, " "); s; s = strtok (NULL, " ")) {
		unsigned long code;
		if (atob (&code, s, 10) && code == 0x15) /* XXX ethernet err */
		    neterr = 1;
	    }
	}
	else
	    /* cannot distinguish test failures, assume the worst */
	    neterr = 1;
    }

    SBD_DISPLAY ("NETI", CHKPNT_NETI);
    init_net (!neterr);
#endif

    SBD_DISPLAY ("HSTI", CHKPNT_HSTI);
    histinit ();

    SBD_DISPLAY ("SYMI", CHKPNT_SYMI);
    syminit ();

#ifdef DEMO
    SBD_DISPLAY ("DEMO", CHKPNT_DEMO);
    demoinit ();
#endif

    SBD_DISPLAY ("MACH", CHKPNT_MACH);
#ifdef SR_FR
    initial_sr = enableCU1 () & (SR_FR | SR_CU1);
#else
    initial_sr = enableCU1 () & SR_CU1;
#endif
    machtype = getmachtype ();

    SBD_DISPLAY ("SBDE", CHKPNT_SBDE);
    initial_sr |= sbdenable (machtype);
#if LOCAL_MEM != 0
    initial_sr |= SR_BEV;
#endif
#ifdef SR_FR
    Status = initial_sr & ~SR_FR; /* don't confuse naive clients */
#endif

    SBD_DISPLAY ("LOGO", CHKPNT_LOGO);
    printf ("\n%s version %s [%s,%s", 
	    _pmon_in_ram ? "PRAM" : "PMON",
	    vers, sbdgetname (), endian);

#ifdef FLOATINGPT
    if (initial_sr & SR_CU1)
      printf (",FP");
#endif
#ifdef INET
    printf (",NET");
#endif

#if 0
    printf ("], LSI LOGIC Corp. %s\n", date);
#else
    printf ("]\nAlgorithmics Ltd. %s\n", date);
#endif
#if defined(P4000) || defined(P4032)  || defined(P5064) || defined(P6032) || defined(P6064) || defined(MIDAS)
    /* Standard Algorithmics warranty applies to supported boards */
    printf ("This software is not subject to copyright and may be freely copied.\n");
#else
    printf ("This is free software, and comes with ABSOLUTELY NO WARRANTY,\n");
    printf ("you are welcome to redistribute it without restriction.\n");
#endif

    sbdmachinfo ();

    if (machtype)
	printf ("CPU type %s%d.", (machtype/1000 == 33) ? "LR" : "R",
		machtype);
    else
	printf ("CPU type UNKNOWN.");

    printf ("  Rev %d.%d.", ((word)Prid >> 4) & 0x0f, (word)Prid & 0x0f);

#define ONEMHZ		(1000 * 1000)
#define DP		2    		/* 2 decimal places for user */

    freq = sbdpipefreq (); /* get the CPU pipeline clock */
    freq += fraction[DP] / 2;
    frac = (freq % ONEMHZ) / fraction[DP];
    if (frac != 0) 
      printf ("  %d.%0*d", freq / ONEMHZ,  DP, frac);
    else if (freq)
      printf ("  %d", freq / ONEMHZ);
    else
      printf ("  ??");
    printf (" MHz");

    freq = sbdcpufreq (); /* get the CPU external clock */
    freq += fraction[DP] / 2;
    frac = (freq % ONEMHZ) / fraction[DP];
    if (frac != 0) 
      printf ("/%d.%0*d", freq / ONEMHZ,  DP, frac);
    else if (freq)
      printf ("/%d", freq / ONEMHZ);
    else
      printf ("/??");
    printf (" MHz.\n");

#define MEG	(1024 * 1024)
    if (memsize % MEG != 0)
      printf ("Memory size %3d KB.\n", memsize / 1024);
    else
      printf ("Memory size %3d MB.\n", memsize / MEG);

#ifdef R4000
    {
	extern int mips_icache_size, mips_icache_linesize;
	extern int mips_dcache_size, mips_dcache_linesize;
	extern int mips_scache_size, mips_scache_linesize;
	extern int mips_tcache_size, mips_tcache_linesize;
	extern int mips_icache_ways, mips_dcache_ways;
	extern int mips_scache_ways, mips_tcache_ways;
	extern int mips_scache_split, mips_scache_discontig;

	printf ("Icache size %3d KB, %2d/line",
		mips_icache_size / 1024, mips_icache_linesize);
	if (mips_icache_ways > 1)
	    printf (" (%d way)", mips_icache_ways);
	printf ("\n");

	printf ("Dcache size %3d KB, %2d/line",
		mips_dcache_size / 1024, mips_dcache_linesize);
	if (mips_dcache_ways > 1)
	    printf (" (%d way)", mips_dcache_ways);
	printf ("\n");

	if (mips_scache_size > 0) {
	    unsigned int size = mips_scache_size;
	    if (mips_scache_split || mips_scache_discontig)
		size *= 2;


	    if (size % MEG != 0)
	      printf ("Scache size %3d KB", size / 1024);
	    else
	      printf ("Scache size %3d MB", size / MEG);
	    printf (", %2d/line",
		    mips_scache_linesize,
		    mips_scache_split ? ", split" : 
		    mips_scache_discontig ? ", discontiguous" :
		    "");
	    if (mips_scache_ways > 1)
		printf (" (%d way)", mips_scache_ways);
	    printf ("\n");
	}

	if (mips_tcache_size > 0) {
	    if (mips_tcache_size % MEG != 0)
	      printf ("Tcache size %3d KB", mips_tcache_size / 1024);
	    else
	      printf ("Tcache size %3d MB", mips_tcache_size / MEG);
	    printf (", %2d/line", mips_tcache_linesize);
	    if (mips_tcache_ways > 1)
		printf (" (%d way)", mips_tcache_ways);
	    printf ("\n");
	}

    }
#else
    {
	extern int icache_size;
	extern int dcache_size;
	if (icache_size >= 1024)
	  printf ("Icache size %3d KB.\n", icache_size / 1024);
	else
	  printf ("Icache size %3d.\n", icache_size);
	if (dcache_size >= 1024)
	  printf ("Dcache size %3d KB.\n", dcache_size / 1024);
	else
	  printf ("Dcache size %3d.\n", dcache_size);
    }
#endif

    printf ("\n\n");

    for (i = 0; i < MAX_BPT; i++)
	Bpt[i].addr = NO_BPT;
    BptTmp.addr = NO_BPT;
    BptTrc.addr = NO_BPT;
    BptTrcb.addr = NO_BPT;
    trace_mode = TRACE_NO;

    _mips_watchpoint_init ();

    Epc = (word) CLIENTPC;
    Gpr[29] = clienttos ();

#if defined(R4000) && defined(FLOATINGPT)
    if (machtype == 4640 || machtype == 4650) {
	int r;
#if __mips >= 3
	/* set m.s. half of float registers to d.p. NaN */
	for (r = R_F0; r <= R_F31; r++)
	    DBGREG[r] = 0x7ff00bad00000000LL;
#else
	/* set odd float registers to d.p. NaN */
	for (r = R_F1; r <= R_F31; r+=2)
	    DBGREG[r] = 0x7ff00bad;
#endif
    }
#endif

#if defined(FLOATINGPT) && !defined(NEWFP)
    c1dat = (struct c1state *)malloc (_fpstatesz ());
    _fpinit (c1dat);
#endif

#ifdef ENB_SHRC
    SBD_DISPLAY ("SHRC", CHKPNT_SHRC);
    shrc(adr);
#else
    s = getenv ("autoboot");
    if (failure) {
	printf ("NOTE: power-on tests reported: %s\n", failure);
	if (s)
	  printf ("      autoboot skipped\n");
#ifdef INET
	if (neterr)
	  printf ("      network disabled\n");
#endif
	return;
    }
    autorun (s);
#endif
}

/*
 * expand(cmdline) - expand environment variables
 * entry:
 *	char *cmdline pointer to input command line
 * returns:
 *	pointer to static buffer containing expanded line.
 */
static char *expand(cmdline)
char *cmdline;
{
  char *ip, *op, *v;
  char var[256];
  static char expline[LINESZ + 8];
  extern char *getenv ();

  if (!strchr (cmdline, '$'))
    return cmdline;

  ip = cmdline;
  op = expline;
  while (*ip) {
    if (op >= &expline[sizeof(expline) - 1]) {
	printf ("Line too long after expansion\n");
	return (0);
    }

    if (*ip != '$') {
      *op++ = *ip++;
      continue;
    }

    ip++;
    if (*ip == '$') {
      *op++ = '$';
      continue;
    }

    /* get variable name */
    v = var;
    if (*ip == '{') {
      /* allow ${xxx} */
      ip++;
      while (*ip && *ip != '}') {
	*v++ = *ip++;
      }
      if (*ip && *ip != '}') {
	printf ("Variable syntax\n");
	return (0);
      }
      ip++;
    }
    else {
      /* look for $[A-Za-z0-9]* */
      while (isalpha(*ip) || isdigit(*ip))
	*v++ = *ip++;
    }

    *v = 0;
    if (!(v = getenv (var))) {
      printf ("Undefined variable %s\n", var);
      return (0);
    }

    if (op + strlen(v) >= &expline[sizeof(expline) - 1]) {
	printf ("Line too long after expansion\n");
	return (0);
    }

    while (*v)
      *op++ = *v++;
  }
  *op = '\0';
  return (expline);
}

/*************************************************************
 *  do_cmd(p)
 *      execute a command, the string pointed to by p.
 *      warning: p does get written to
 */
do_cmd (p)
     char           *p;
{
    char           *av[MAX_AC];	/* argument holder */
    word            ac;		/* # of arguments */
    char           *cmdlist[20], *t, tmp[11];
    int             i, nc, j, c, rptcmd;

    repeating_cmd = 0;
    if (!*p || strempty (p)) {	/* blank */
	rptcmd = matchenv ("rptcmd");
	if (rptcmd) {		/* repeat requested */
	    repeating_cmd = 1;
	    t = gethistn (histno - 1);
	    if (rptcmd == 1)
		strcpy (p, t);	/* all cmds */
	    else if (rptcmd == 2) {	/* trace only */
		if (wordsz (t) > 10)
		    return;
		getword (tmp, t);
		if (strequ (tmp, "t") || strequ (tmp, "to"))
		    strcpy (p, tmp);
		else
		    return;
	    } else {
		printf ("bad rptcmd value [%s]\n", getenv ("rptcmd"));
		return;
	    }
	} else
	    return;
    }

    if (!(p = expand (p)))
      return;

    nc = 0;
    cmdlist[nc++] = p;
    for (; *p;) {
	c = *p;
	if (c == '\'' || c == '"') {
	    p++;
	    while (*p && *p != c)
		++p;
	    if (*p)
		p++;
	} else if (c == ';') {
	    *p++ = 0;
	    cmdlist[nc++] = p;
	} else
	    p++;
    }

    for (j = 0; j < nc; j++) {
	ac = argvize (av, cmdlist[j]);
	if (ac > 0) {
	    for (i = 0; CmdTable[i].name != 0; i++)
		if (strequ (CmdTable[i].name, av[0]))
		    break;
	    if (CmdTable[i].name != 0) {
		int stat = -1;

		if( repeating_cmd && !CmdTable[i].repeat )
		{
		  /*
		   * This command isn't supposed to repeat.  Just return.
		   */

		  repeating_cmd = 0;
		  return;
		}
		
		if (ac < CmdTable[i].minac)
		  printf ("%s: not enough arguments\n", CmdTable[i].name);
		else if (ac > CmdTable[i].maxac)
		  printf ("%s: too many arguments\n", CmdTable[i].name);
		else
		  stat = (CmdTable[i].func) (ac, av);
		if (stat < 0)
		  printf ("usage: %s %s\n", CmdTable[i].name, CmdTable[i].opts);
		if (stat != 0)
		  break;	/* skip commands following ';' */
	    } else {
		printf ("%s: Command not found.\n", av[0]);
		break;
	    }
	}
    }
}

/*************************************************************
 *  help(ac,av)
 */
help (ac, av)
     int             ac;
     char           *av[];
{
    int             i, j, namemax, optsmax, descmax, len;
    int             ln, siz;

    namemax = optsmax = descmax = 0;
    for (i = 0; CmdTable[i].name != 0; i++) {
	len = strlen (CmdTable[i].name);
	if (len > namemax)
	    namemax = len;
	len = strlen (CmdTable[i].opts);
	if (len > optsmax)
	    optsmax = len;
	len = strlen (CmdTable[i].desc);
	if (len > descmax)
	    descmax = len;
    }

    siz = moresz;
    ioctl (STDIN, CBREAK, NULL);

    ln = siz;
    if (ac >= 2) {		/* extended help */
	if (strequ (av[1], "*")) {	/* all commands */
	    for (i = 0; CmdTable[i].name != 0; i++) {
		if (prhelp (i, &ln, siz, namemax, optsmax))
		    break;
	    }
	} else {		/* specific commands */
	    for (j = 1; j < ac; j++) {
		for (i = 0; CmdTable[i].name != 0; i++) {
		    if (strequ (CmdTable[i].name, av[j]))
			break;
		}
		if (CmdTable[i].name) {
		    if (prhelp (i, &ln, siz, namemax, optsmax))
			break;
		} else
		    printf ("%s: command not found\n", av[j]);
	    }
	}
    } else {			/* general help only */
	for (i = 0; CmdTable[i].name != 0; i++) {
	    printf ("%*s  %-*s", namemax, CmdTable[i].name, descmax,
		    CmdTable[i].desc);
	    if (i % 2 != 0)
		printf ("\n");
	    else
		printf ("   ");
	}
	if (i % 2 != 0)
	    printf ("\n");
    }
    return (0);
}

/*************************************************************
 *  prhelp(n,lnp,siz,namemax,optsmax)
 */
prhelp (n, lnp, siz, namemax, optsmax)
     int             n, *lnp, siz, namemax, optsmax;
{
    const Optdesc        *p;
    int             i;

    sprintf (prnbuf, "%*s  %-*s    %s", namemax, CmdTable[n].name, optsmax,
	     CmdTable[n].opts, CmdTable[n].desc);
    if (more (prnbuf, lnp, siz))
	return (1);

    p = CmdTable[n].optdesc;
    if (p != 0) {
	for (i = 0; p[i].name; i++) {
	    sprintf (prnbuf, "%*s  %15s    %s", namemax, "",
		     p[i].name, p[i].desc);
	    if (more (prnbuf, lnp, siz))
		return (1);
	}
    }
    return (0);
}

/*************************************************************
 *  shrc(adr)
 */
shrc (adr)
     char           *adr;
{
    int             c;
    char            buf[LINESZ];

    for (;;) {
	c = getln (&adr, buf);
	if (c == 0)
	    break;
	do_cmd (buf);
    }
}

/*************************************************************
 *  getln(adr,p)
 */
getln (adr, p)
     unsigned char **adr, *p;
{
    unsigned int    c;

    for (;;) {
	c = **adr;
	if (c == 0xff)
	    c = 0;
	if (c == 0)
	    break;
	(*adr)++;
	if (c == '\n')
	    break;
	*p++ = c;
    }
    *p = 0;
    return (c);
}

/*************************************************************
 *  word no_cmd(ac,av)
 */
word 
no_cmd (ac, av)
     int             ac;
     char           *av[];
{

    printf ("The %s command cannot be invoked by name.\n", av[0]);
    return (1);
}

/*************************************************************
 *  closelst(lst)
 */
closelst (lst)
     int             lst;
{
    int             i, x;
    struct termio   tbuf;

    switch (lst) {
    case 0:
	if (curlst == 0)
	    return;
	x = *curlst;
	*curlst = 0;
	break;
    case 1:
	x = pmlst;
	pmlst = 0;
	break;
    case 2:
	x = clilst;
	clilst = 0;
	/* reset client terminal state to "sane" value */
	ioctl (STDIN, TCGETA, &tbuf);
	ioctl (STDIN, SETSANE, NULL);
	ioctl (STDIN, TCGETA, &clitbuf);
	ioctl (STDIN, TCSETAW, &tbuf);
	break;
    }

    for (i = 0; i < OPEN_MAX; i++) {
	if (x & 1 && i > 2)
	    close (i);
	x >>= 1;
    }
}

/*************************************************************
 *  swlst(lst)
 */
swlst (lst)
     int             lst;
{

    switch (lst) {
    case 1:
	if (curlst != &pmlst) {
	    if (_pmon_in_ram) {
		SBD_DISPLAY ("PRAM", CHKPNT_PRAM);
	    }
	    else {
		SBD_DISPLAY ("PMON", CHKPNT_PMON);
	    }
	    /* save client terminal state and set PMON default */
	    ioctl (STDIN, TCGETA, &clitbuf);
	    ioctl (STDIN, SETSANE, NULL);
	    curlst = &pmlst;
	}
	break;
    case 2:

	if (curlst != &clilst) {
	    SBD_DISPLAY ("RUN ", CHKPNT_RUN);
	    /* restore client terminal state */
	    ioctl (STDIN, TCSETAF, &clitbuf);
	    curlst = &clilst;
	}
	break;
    }
}


#ifdef R4000
struct cerrfifo	cerrfifo;

void
cerrpoll ()
{
    static const char * const msg ="\r\nWarning: correctable cache error, CacheErr=0x";
    static int incerrpoll = 0;
    volatile struct cerrfifo *ce;
    char xbuf[10];

    if (incerrpoll)
	return;
    incerrpoll = 1;

    ce = (struct cerrfifo *)((unsigned int)&cerrfifo | K1BASE);
    while (ce->ce_out != ce->ce_in) {
	unsigned int val = ce->ce_buf[ce->ce_out];
	ce->ce_out = (ce->ce_out + 1) & CERR_FIFO_MASK;
	write (2, msg, strlen (msg));
	(void) btoa (xbuf, val, 16);
	write (2, xbuf, strlen (xbuf));
	write (2, "\r\n", 2);
    }

    incerrpoll = 0;
}
#endif


#ifdef SABLE
/*************************************************************
 *  mread(fd,buf,size)
 */
mread (fd, buf, size)
     int             fd, size;
     char           *buf;
{
    static char    *madr = (char *)0x80080000;
    int             c, i;

    for (i = 0;;) {
	c = *madr;
	if (c == 0)
	    break;
	madr++;
	if (c == CNTRL ('C'))
	    break;
	if (c == '\n')
	    break;
	buf[i++] = c;
	if (i >= size)
	    break;
    }
    write (1, ".", 1);
    return (i);
}
#endif
