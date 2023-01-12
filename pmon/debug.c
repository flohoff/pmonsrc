/* $Id: debug.c,v 1.9 2000/03/28 00:20:53 nigel Exp $ */
/*************************************************************
 * This module provides dbx support, permitting PMON to be used as
 * an execution vehicle when source-level debugging with dbx.

 To use this feature perform the following steps:

 1. Create the file /etc/premote.pdbx which should
 contain the following single line.

 pmon:dv=/dev/tty1:br#9600

 2. Create the file ~/.dbxinit which should contain the following,

 set $pdbxport="pmon"
 set $usesockets=0
 set $manual_load=1

 3. Download the program to the target system

 4. Invoke dbx using the following command,

 dbx -prom <obj_file>

 5. Optionally set breakpoints e.g., "bp main".

 6. Start execution using the dbx command "run".

 7. Type "debug" on the target system's console.

 *************************************************************/

/*
 * Messages are transmitted over an RS232 serial link, and are of the
 * form:
 
 *      ^V type+len sequence# data checkSum
 
 * These messages are transmitted by the routine putpkt and received by
 * getpkt.
 */

#include "sys/types.h"
#include "mips/cpu.h"
#include "watchpoint.h"

#include "stdio.h"
#include "termio.h"
#include "mips.h"
#include "pmon.h"
#include "setjmp.h"

/* All packets start with a SYN */
#define SYN			'\026'

/* MIPS SPP signal numbers */
#define	SPP_SIGTRAP		5
#define	SPP_SIGINT		2
#define	SPP_SIGSEGV		11
#define	SPP_SIGBUS		10
#define	SPP_SIGILL		4
#define	SPP_SIGFPE		8
#define	SPP_SIGTERM		15

#define WSTOPPED		0177
#define WSETSTOP(stat, sig)	*(stat) = ((sig) << 8) | WSTOPPED
#define WSETEXIT(stat,ret,sig)	*(stat) = ((ret) << 8) | (sig)

#ifndef EFAULT
#define EFAULT			14
#define EINVAL			22
#endif

#define DATA	0x00
#define ACKPKT	0x20
#define nextseq(x)		(((x) + 1) & 63)

#if defined(RTC) || defined(INET)
#define TIMEOUT 5	/* timeout in seconds */
#else
#define TIMEOUT 300000	/* timeout in loops */
#endif

const Optdesc         debug_opts[] =
{
    {"-s", "don't set client sp"},
    {"-v", "report protocol errors"},
    {"-V", "verbose mode"},
    {"-h <port>", "port to communicate with host"},
    {"-- <args>", "args to be passed to client"},
    {0}};

static int             txseq;
static int             rxseq;
static int             lastwasget;
static int             hostfd;
static int             vflag;
static int             Vflag;
static jmp_buf         pktstart;
static jmp_buf         timeout;
static int	       newgdb;

#define MAXMSG	80

extern char     clientcmd[LINESZ];
extern char    *clientav[MAX_AC];
extern int	clientac;

extern jmp_buf	gobuf;
extern int	gobufvalid;

static int
#ifdef __GNUC__
inline
#endif
readc (char *msg, unsigned int state);

/*************************************************************
 *  debug(ac,av)
 *      The 'debug' command
 */
debug (ac, av)
     int             ac;
     char           *av[];
{
    struct termio   tbuf;
    char           *hostport;
    int             i, j, sflag;
    extern int	    optind;
    extern char    *optarg;
    int             c;

    strcpy (clientcmd, av[0]);
    strcat (clientcmd, " ");

    hostport = getenv ("dbgport");
    if (!hostport)
	hostport = getenv ("hostport");

    vflag = Vflag = sflag = 0;

    optind = 0;
    while ((c = getopt (ac, av, "vVsh:")) != EOF)
      switch (c) {
      case 'v':
	  vflag++; break;
      case 'V':
	  Vflag++; break;
      case 's':
	  sflag++; break;
      case 'h':
	  hostport = optarg; break;
      default:
	  return (-1);
      }

    while (optind < ac) {
	strcat (clientcmd, av[optind++]);
	strcat (clientcmd, " ");
    }

    if (Vflag)
	printf ("hostport=%s\n", hostport);
    hostfd = open (hostport, 0);
    if (hostfd == -1) {
	printf ("can't open %s\n", hostport);
	return (1);
    }
    if (Vflag)
	printf ("hostfd=%d\n", hostfd);

    /* need to catch ^C on debug channel using interrupts */
    ioctl (hostfd, TCGETA, &tbuf);
    tbuf.c_iflag = ISTRIP;
    tbuf.c_oflag = 0;
    tbuf.c_lflag = 0;
    tbuf.c_cc[VINTR] = '\03';
    tbuf.c_cc[4] = 1; /*???*/
    ioctl (hostfd, TCSETAF, &tbuf);

#ifndef OLDVERSION
    /* wait for "start" signal, this make initialisation much quicker */
    SBD_DISPLAY ("WAIT", CHKPNT_WAIT);
    do {
	c = readc (NULL, -1);
	if (c < 0 && Vflag)
	    putchar ('.');
    } while (c != '\r');
#endif
    write (hostfd, "\r", 1);

    txseq = rxseq = lastwasget = 0;

    if (!sflag) {
	clientac = argvize (clientav, clientcmd);
	/* set client's a0 (argc) & a1 (argv) and stack */
	Gpr[29] = clienttos ();
	initstack (clientac, clientav, 1);
	closelst (2);
	clrhndlrs ();
	Status = initial_sr;
#ifdef Icr
	/* RM7000 interrupt extensions */
	Icr = IplLo = IplHi = 0;
#endif
	sbdenable (getmachtype ());  /* set up i/u hardware */
#ifdef FLOATINGPT
	Fcr = 0;		/* clear outstanding exceptions / enables */
#endif
    }

    clrbpts ();			/* clear user breakpoints */
    dbgmode ();
}

const char * const wpflags[] = {"",  "w",  "r", "rw",
				"x", "xw", "xr", "xrw"};

/*************************************************************
 *  dbgmode(type) enter dbx mode
 */
dbgmode ()
{
    char            rxstr[MAXMSG], *rxarg[8];
    int             req, sig, stat, ac, exited;
    int		    hit = 0; 
    vaddr_t	    hitaddr = 0;
    size_t	    hitlen = 0;

    SBD_DISPLAY ("RDBG", CHKPNT_RDBG);

    if (setjmp (&gobuf) == 0) {
	/* normal */
	gobufvalid = 1;
	exited = 0;
    }
    else {
	/* program called cliexit() */
	gobufvalid = 0;
	exited = 1;
    }

    switch (trace_mode) {
    case TRACE_DS:
	req = 's';
	break;
    case TRACE_DC:
	req = 'c';
	break;
    default:
	req = 'b';
	break;
    }

    if (trace_mode != TRACE_NO)
	hit = _mips_watchpoint_hit (&hitaddr, &hitlen);
	
    sig = SPP_SIGTRAP;
    if (req != 'b')
      switch (Cause & CAUSE_EXCMASK) {
      case CEXC_TLBL:
      case CEXC_TLBS:
	  sig = SPP_SIGSEGV;
	  break;
      case CEXC_MOD:
      case CEXC_ADEL:
      case CEXC_ADES:
      case CEXC_IBE:
      case CEXC_DBE:
	  sig = SPP_SIGBUS;
	  break;
      case CEXC_BP:
	  sig = SPP_SIGTRAP;
	  break;
      case CEXC_SYS:
      case CEXC_RI:
      case CEXC_CPU:
	  sig = SPP_SIGILL;
	  break;
      case CEXC_OVF:
	  sig = SPP_SIGFPE; /* !! */
	  break;
#ifdef R4000
      case CEXC_TRAP:
      case CEXC_WATCH:
	  sig = SPP_SIGTRAP;
	  break;
      case CEXC_FPE:
	  sig = SPP_SIGFPE;
	  break;
      case CEXC_VCEI:
      case CEXC_VCED:
	  sig = SPP_SIGTERM;
	  break;
#endif /* R4000 */
      }
    if (exited)
	WSETEXIT (&stat, 0, 0);
    else
	WSETSTOP (&stat, sig);

    sprintf (rxstr, "0x1 %c 0x0 0x%x", req, stat);
    if (newgdb) {
	/* add pc, s8, sp */
	sprintf (rxstr + strlen (rxstr), " 0x%x 0x%x 0x%x",
		 (long)Epc, (long)Gpr[R_FP], (long)Gpr[R_SP]);
	if (hit)
	    /* add wpaddr, wpflags, wplen */
	    sprintf (rxstr + strlen (rxstr), " 0x%x %s%d", 
		     hitaddr, wpflags[hit & 7], hitlen);
    }

    putmsg (rxstr);
    for (;;) {
	getmsg (rxstr);
	ac = argvize (rxarg, rxstr);
	do_req (ac, rxarg);
    }
}

/*************************************************************
 *  putmsg(msg)  send msg, including wait for the ACK
 */
putmsg (msg)
     char           *msg;
{
    int             type, ts, ns;

    ns = nextseq (txseq);

 resend:
    /* send the message */
    putpkt (txseq, msg);

    /* wait for other end to send acknowledge, or a timeout */
    for (;;) {
	if ((type = getpkt (&ts, 0)) == ACKPKT && ts == ns) 
	    break;

	if (vflag && type >= 0)
	  printf("PutMsg: got type=%s seq=%d, wanted type=ACK seq=%d\r\n",
		 (type == ACKPKT) ? "ACK" : "DATA", ts, ns);

	if (type == -2) {
	    if (lastwasget)
	      /* timeout: the other end may have lost our last Rx ACK, resend it */
	      putpkt (rxseq, 0);
	    goto resend;
	}

	if (type == ACKPKT && ts == txseq)
	  /* ACK for previous packet: resend current packet */
	  goto resend;
    }

    txseq = ns;
    lastwasget = 0;
    if (Vflag)
	printf ("\r\n");
}

/*************************************************************
 *  getmsg(msg)  get msg, including send the ACK
 */
getmsg (msg)
     char           *msg;
{
    int             type, ts;

    /* wait for a data packet */
    for (;;) {
	if ((type = getpkt (&ts, msg)) == DATA && ts == rxseq)
	    break;

	if (vflag && type >= 0)
	  printf("GetMsg: got type=%s seq=%d msg=<%s>, wanted type=DATA seq=%d\r\n",
		 (type == ACKPKT) ? "ACK" : "DATA", ts, msg, rxseq);

	if (type < 0)
	  /* checksum/timeout error: reacknowledge last rx to force resend */
	  putpkt (rxseq, 0);

	/* quietly ignore other packets */
    }

    /* acknowledge this packet */
    rxseq = nextseq (rxseq);
    putpkt (rxseq, 0);
    lastwasget = 1;
}

/*************************************************************
 *  putpkt(seq,msg) send a packet, if msg == 0, type = ACK
 */
putpkt (seq, msg)
     int             seq;	/* sequence number to be sent */
     char           *msg;	/* message to be sent */
{
    int             len, type, type_len, csum;
    char            tmp[80], *t = tmp;
    
    if (Vflag)
      printf ("PutPkt: seq=%d msg=<%s>\r\n", seq, msg ? msg : "ACK");

    if (msg) {
	type = DATA;
	len = strlen (msg);
    } else {
	type = ACKPKT;
	len = 0;
    }

    type_len = type | (len >> 6);
    len &= 0x3f;
    csum = 0;

#define addc(c) (*t++ = (c), csum += (c))

    /* header */
    *t++ = SYN;
    addc (type_len + '@');
    addc (len + '@');
    addc (seq + '@');

    /* message */
    if (msg)
      for (; *msg; msg++)
	addc (*msg);

#undef addc

    /* checksum */
    *t++ = ((csum >> 12) & 0x3f) + '@';
    *t++ = ((csum >> 6) & 0x3f) + '@';
    *t++ = (csum & 0x3f) + '@';
    /**t++ = '\n';*/

    write (hostfd, tmp, t - tmp);
}


#define STATE_START	0
#define STATE_TYPLEN1	1
#define STATE_LEN2	2
#define STATE_SEQ	3
#define STATE_MSG	4
#define STATE_CSUM1	5
#define STATE_CSUM2	6
#define STATE_CSUM3	7
#define STATE_END	8

static const char * const statenames[] = {
    "START", "TYPLEN1", "LEN2", "SEQ", "MSG", "CSUM1", "CSUM2", "CSUM3", "END"
};

/*************************************************************
 *  readc(msg, state)
 */
static 
#ifdef __GNUC__
inline
#endif
int
readc (msg, state)
    char           *msg;
    unsigned int    state;
{
    char            ch;
    int 	    n;
    
    ioctl (hostfd, FIONREAD, &n);
    if (n <= 0 && (state > STATE_START || !msg)) {
	/* in packet or waiting for an ACK: timeout the transfer */
#if defined(RTC) || defined(INET)
	extern unsigned long __time ();
	unsigned long timelimit = __time (0) + (msg ? TIMEOUT * 2 : TIMEOUT);
#else
	int i = TIMEOUT;
#endif

	do {
#if defined(RTC) || defined(INET)
	    if (__time (0) > timelimit)
#else
	    if (i-- <= 0) 
#endif
	      return (-1);
	    ioctl (hostfd, FIONREAD, &n);
	} while (n <= 0);
    }

    read (hostfd, &ch, 1);
    return (ch);
}


/*************************************************************
 *  getpkt(seq,msg) get a packet as a string, returns type
 */
getpkt (seq, msg)
     int            *seq;	/* received sequence number */
     char           *msg;	/* destination for message */
{
    unsigned int    state;
    int		    type;
    char	    xmsg[MAXMSG], *rmsg;
    int             len, rlen;
    int		    csum, rsum;
    int             ch;

    if (!(rmsg = msg))
	rmsg = xmsg;

    state = STATE_START;
    while (state != STATE_END) {

	if ((ch = readc (msg, state)) < 0) {
	    /* timeout: return error to getmsg/putmsg */
	    if (vflag)
	      printf ("GetPkt: state %s, timeout\r\n", statenames[state]);
	    return (-2);
	}

	if (ch == SYN) {
	    /* restart packet on a SYN */
	    if (state != STATE_START && vflag)
	      printf ("GetPkt: state %s, restart\r\n", statenames[state]);
	    state = STATE_TYPLEN1;
	    csum = 0;
	    rsum = 0;
	    rlen = 0;
	    continue;
	}

	switch (state) {
	case STATE_START:
	    /* waiting for SYN */
	    break;

	case STATE_TYPLEN1:
	    if (ch < '@')
	      goto badchar;
	    rsum += ch;
	    type = ch & ACKPKT;
	    len = ch & 0x1f;
	    state++;
	    break;

	case STATE_LEN2:
	    if (ch < '@')
	      goto badchar;
	    rsum += ch;
	    len = (len << 6) + (ch - '@');
	    if (len >= MAXMSG) {
		if (vflag)
		  printf ("GetPkt: bad length %d\r\n", len);
		return (-1);
	    }
	    state++;
	    break;

	case STATE_SEQ:
	    if (ch < '@')
	      goto badchar;
	    rsum += ch;
	    *seq = ch - '@';
	    state += (len == 0) ? 2 : 1;
	    break;

	case STATE_MSG:
	    rsum += ch;
	    rmsg[rlen] = ch;
	    if (++rlen == len)
	      state++;
	    break;

	case STATE_CSUM1:
	case STATE_CSUM2:
	case STATE_CSUM3:
	    if (ch < '@')
	      goto badchar;
	    csum = (csum << 6) + (ch - '@');
	    state++;
	    break;
	}
    }
	
    if ((rsum & 0x3ffff) != csum) {
	if (vflag)
	  printf ("GetPkt: csum error tx=%x rx=%x\r\n", csum, rsum);
	return (-1);
    }

    rmsg[rlen] = '\0';

    if (Vflag)
      printf("GetPkt: seq=%d type=%s msg=<%s>\r\n", *seq,
	     (type == ACKPKT) ? "ACK" : "DATA", rmsg);

    return (type);

 badchar:
    if (vflag)
      printf ("GetPkt: state %s, bad char=0x%x\r\n", statenames[state], ch);
    return (-1);
}



static sreg_t *
mapdbgreg (adr)
    unsigned long adr;
{
    if (adr < 32) {
	adr = R_ZERO + adr;
#ifdef FLOATINGPT
    } else if (adr < 64) {
	adr = R_F0 + adr - 32;
#if __mips >= 3
	if (!(Status & SR_FR))
	    adr &= ~1;		/* odd registers "don't exist" */
#endif
#endif
    } else {
	switch (adr) {
	case 96:
	    adr = R_EPC;
	    break;
	case 97:
	    adr = R_CAUSE;
	    break;
	case 98:
	    adr = R_HI;
	    break;
	case 99:
	    adr = R_LO;
	    break;
#ifdef FLOATINGPT
	case 100:
	    adr = R_FCR;
	    break;		/* fcsr */
	case 101:
	    adr = R_ZERO;
	    break;		/* feir */
#endif
	default:
	    adr = R_ZERO;
	    break;
	}
    }
    return &DBGREG[adr];
}


static sreg_t
getdbgreg (adr, size)
    unsigned long adr;
    int size;
{
    sreg_t *rp = mapdbgreg (adr);

#if defined(FLOATINGPT) && __mips >= 3
    if (adr >= 32 && adr < 64 && size == 4) {
	/* pick correct half of 64-bit register */
	unsigned int *frp = (unsigned int *)rp;
#if MIPSEB
	adr = (adr & 1) ^ 1;	/* register order != memory order */
#else
	adr = (adr & 1);
#endif
	return frp[adr];
    }
#endif

    return *rp;
}


static sreg_t
putdbgreg (adr, size, val)
    unsigned long adr;
    int size;
    sreg_t val;
{
    sreg_t *rp = mapdbgreg (adr);
    sreg_t oval;

#if defined(FLOATINGPT) && __mips >= 3
    if (adr >= 32 && adr < 64 && size == 4) {
	/* pick correct half of 64-bit register */
	unsigned int *frp = (unsigned int *)rp;
#if MIPSEB
	adr = (adr & 1) ^ 1;	/* register order != memory order */
#else
	adr = (adr & 1);
#endif
	oval = frp[adr];
	frp[adr] = val;
	return oval;
    }
#endif

    oval = *rp;
    if (adr != 0)
	*rp = val;
    return oval;
}


/*************************************************************
 *  do_req(ac,av) handle a dbx request
 *      av[0]=pid av[1]=req av[2]=addr av[3]=data
 */
do_req (ac, av)
     int             ac;
     char           *av[];
{
    unsigned long   adr, val32 = 0;
    sreg_t           *rp;
    sreg_t	    dat;
    int             i;
    int		    err = 0;
    char            msg[80];

    if (ac < 4) {
	if (vflag) {
	    printf ("ac=%d: ");
	    for (i = 0; i < ac; i++)
	      printf ("av[%d]=%s ", i, av[i]);
	    printf ("\r\n");
	}
	goto badpkt;
    }

    if (av[1][1])
	goto badpkt;

    atob (&adr, av[2], 0);
#if __mips >= 3
    llatob (&dat, av[3], 0);
#else
    atob (&dat, av[3], 0);
#endif

    switch (av[1][0]) {
    case 'i':			/* read Ispace */
    case 'd':			/* read Dspace */
	if (adr >= K0BASE && adr < K2BASE && (adr & 3) == 0)
	    val32 = load_word (adr);
	else {
	    err = -1;
	    val32 = EFAULT;
	}
	sprintf (msg, "%s %s 0x%x 0x%x", av[0], av[1], err, val32);
	putmsg (msg);
	break;
    case 'h':			/* read half-word */
	if (adr >= K0BASE && adr < K2BASE && (adr & 1) == 0)
	    val32 = load_half (adr);
	else {
	    err = -1;
	    val32 = EFAULT;
	}
	sprintf (msg, "%s %s 0x%x 0x%x", av[0], av[1], err, val32);
	putmsg (msg);
	break;
    case 'p':			/* read byte */
	if (adr >= K0BASE && adr < K2BASE)
	    val32 = load_byte (adr);
	else {
	    err = -1;
	    val32 = EFAULT;
	}
	sprintf (msg, "%s %s 0x%x 0x%x", av[0], av[1], err, val32);
	putmsg (msg);
	break;
#if __mips >= 3
    case 't':			/* read single 64-bit reg */
	sprintf (msg, "%s %s 0x0 0x%llx", av[0], av[1], getdbgreg (adr, 8));
	putmsg (msg);
	break;
#endif
    case 'r':			/* read reg */
	val32 = getdbgreg (adr, 4);
	sprintf (msg, "%s %s 0x0 0x%x", av[0], av[1], val32);
	putmsg (msg);
	break;
#if __mips >= 3
    case 'u':			/* read multiple 64-bit regs */
	for (i = 0; i < 32; i++) {
	    if (adr & 1)
		rp = &DBGREG[i];
	    sprintf (msg, "%s %s 0x0 0x%llx", av[0], av[1], *rp);
	    putmsg (msg);
	    adr >>= 1;
	}
	break;
#endif
    case 'g':			/* read multiple regs */
	for (i = 0; i < 32; i++) {
	    if (adr & 1)
		rp = &DBGREG[i];
	    val32 = *rp;
	    sprintf (msg, "%s %s 0x0 0x%x", av[0], av[1], val32);
	    putmsg (msg);
	    adr >>= 1;
	}
	break;

    case 'I':			/* write Ispace */
    case 'D':			/* write Dspace */
	if (adr >= K0BASE && adr < K2BASE && (adr & 3) == 0) {
	    val32 = load_word (adr);
	    store_word (adr, dat);
	} else {
	    err = -1;
	    val32 = EFAULT;
	}
	sprintf (msg, "%s %s 0x%x 0x%x", av[0], av[1], err, val32);
	putmsg (msg);
	break;
    case 'H':			/* write half-word */
	if (adr >= K0BASE && adr < K2BASE && (adr & 1) == 0) {
	    val32 = load_byte (adr);
	    store_half (adr, dat);
	} else {
	    err = -1;
	    val32 = EFAULT;
	}
	sprintf (msg, "%s %s 0x%x 0x%x", av[0], av[1], err, val32);
	putmsg (msg);
	break;
    case 'P':			/* write byte */
	if (adr >= K0BASE && adr < K2BASE) {
	    val32 = load_byte (adr);
	    store_byte (adr, dat);
	} else {
	    err = -1;
	    val32 = EFAULT;
	}
	sprintf (msg, "%s %s 0x%x 0x%x", av[0], av[1], err, val32);
	putmsg (msg);
	break;
#if __mips >= 3
    case 'T':			/* write 64-bit reg */
	{
	    sprintf (msg, "%s %s 0x0 0x%llx", av[0], av[1], 
		     putdbgreg (adr, 8, dat));
	    putmsg (msg);
	}
	break;
#endif
    case 'R':			/* write reg */
	val32 = putdbgreg (adr, 4, (long)dat);
	sprintf (msg, "%s %s 0x0 0x%x", av[0], av[1], val32);
	putmsg (msg);
	break;

    case 's':			/* step */
	/* optional addr */
	if (adr != 1)
	    DBGREG[R_EPC] = adr;
	sstep ();
	break;
    case 'c':			/* continue */
	/* optional addr */
	if (adr != 1)
	    DBGREG[R_EPC] = adr;
#if 0
	trace_mode = TRACE_DC;
	_go ();
#else
	godebug ();
#endif
	break;
    case 'x':			/* exit */
	sprintf (msg, "%s %s 0x0 0x0", av[0], av[1]);
	putmsg (msg);
	trace_mode = TRACE_NO;
	gobufvalid = 0;
	newgdb = 0;
	close (hostfd);
	clrbpts ();
	printf ("Exiting remote debug mode\r\n");
	cliexit ();
	break;

    case 'B':	/* breakpoint set */
    case 'b':	/* breakpoint clear */
	if (ac < 5)
	    goto badpkt;
	switch (av[4][0]) {
	case 'n':	/* return number of watchpoints */
	    i = _mips_watchpoint_howmany ();
	    sprintf (msg, "%s %s 0x0 0x%x", av[0], av[1], i < 0 ? 0 : i);
	    break;
	case 'q':	/* return watchpoint capabilities */
	    i = _mips_watchpoint_capabilities (adr);
	    sprintf (msg, "%s %s 0x0 0x%x", av[0], av[1], i);
	    break;
	case 'u':
	    if (av[1][0] == 'B')
		goto badpkt;
	    /* clear all breakpoints */
	    clrbpts ();
	    sprintf (msg, "%s %s 0x0 0x0", av[0], av[1]);
	    newgdb = 1;
	    break;
	case 'x':
	    i = MIPS_WATCHPOINT_X;
	    goto dobpt;
	case 'w':
	    i = MIPS_WATCHPOINT_W;
	    goto dobpt;
	case 'r':
	    i = MIPS_WATCHPOINT_R;
	    goto dobpt;
	case 'a':
	    i = MIPS_WATCHPOINT_R | MIPS_WATCHPOINT_W;
	    goto dobpt;
	case 'f':
	    i = 0;
	dobpt:
	    if (av[1][0] == 'B') {
		if (!setbpt (i, adr, (long)dat))
		    goto error;
	    }
	    else {
		if (!clrbpt (i, adr, (long)dat))
		    goto error;
	    }
	    sprintf (msg, "%s %s 0x0 0x0", av[0], av[1]);
	    break;
	default:
	    goto badpkt;
	}
	putmsg (msg);
	break;

    default:
    badpkt:
	if (vflag)
	  printf ("unknown request type '%s %s %s %s'\r\n",
		  av[0], av[1], av[2], av[3]);
    error:
	sprintf (msg, "%s %s 0x%x 0x%x", av[0], av[1], -1, EINVAL);
	putmsg (msg);
	break;
    }
}
