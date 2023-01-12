/* $Id: load.c,v 1.14 1999/03/22 16:46:41 nigel Exp $ */
#include "mips.h"
#include "pmon.h"
#include "stdio.h"
#include "termio.h"
#include "set.h"
#include "string.h"
#include "signal.h"

#include "loadfn.h"

#ifndef NO_SERIAL

/* control character used for download */
#define ETX	CNTRL('c')
#define ACK	CNTRL('f')
#define NAK	CNTRL('u')
#define XON	CNTRL('q')
#define XOFF	CNTRL('s')

extern char *heaptop;

static jmp_buf         ldjmpb;		/* non-local goto jump buffer */

#ifdef INET
static void
ldintr (int dummy)
{
    sigsetmask (0);
    longjmp (ldjmpb, 1);
}
#endif


const Optdesc         load_opts[] =
{
    {"-s", "don't clear old symbols"},
    {"-b", "don't clear breakpoints"},
    {"-e", "don't clear exception handlers"},
    {"-a", "don't add offset to symbols"},
    {"-t", "load at top of memory"},
    {"-i", "ignore checksum errors"},
#ifdef FLASH
    {"-f", "load into flash"},
#endif
    {"-n", "don't load symbols"},
    {"-y", "only load symbols"},
    {"-v", "verbose messages"},
    {"-o<offs>", "load offset"},

    {"-c cmdstr", "send cmdstr to host"},
    {"-u<num>", "set baud rate"},
    {"-h<port>", "load from <port>"},

    {0}};

/*************************************************************
 *  load(ac,av)
 */
load (ac, av)
     int             ac;
     char           *av[];
{
    FILE	   *fp;
    struct termio   otbuf, tbuf;
    char           *hostport, *cmdstr;
    int             dlecho, dlproto, dltype, count, n;
    int             len, err, i, j, s, tot, recno, flags, istty, isnet;
    char            recbuf[DLREC], *ep, *baudrate;
    int             errs[DL_MAX];
    unsigned long   offset;
#ifdef FLASH
    void	    *flashbuf;
    int		    flashsize;
#endif
    extern int	    optind;
    extern char    *optarg;
#ifdef INET
    sig_t	    ointr;
#else
    jmp_buf	   *ointr;
#endif
    int		    intr;
    int             c;
#ifdef INET
    int ospl;
#endif

    baudrate = 0;
    count = 0;
    cmdstr = 0;
    hostport = 0;
    flags = 0;
    istty = 0;
    isnet = 0;
    dlecho = OFF;
    dlproto = NONE;
    offset = 0;

    optind = 0;
    while ((c = getopt (ac, av, "sbeatifnyvo:c:u:h:")) != EOF)
      switch (c) {
      case 's':
	  flags |= SFLAG; break;
      case 'b':
	  flags |= BFLAG; break;
      case 'e':
	  flags |= EFLAG; break;
      case 'a':
	  flags |= AFLAG; break;
      case 't':
	  flags |= TFLAG; break;
      case 'i':
	  flags |= IFLAG; break;
#ifdef FLASH
      case 'f':
	  flags |= FFLAG; break;
      case 'w':
	  flags |= WFLAG; break;
#endif
      case 'v':
	  flags |= VFLAG; break;
      case 'o':
	  if (!get_rsa (&dl_offset, optarg))
	    return (-1);
	  break;

      case 'c':
	  cmdstr = optarg; break;
      case 'h':
	  hostport = optarg; break;
      case 'u':
	  baudrate = optarg; break;
      default:
	  return (-1);
      }

    if (optind < ac && !hostport)
      hostport = av[optind++];
    if (optind < ac)
      return (-1);

#ifdef FLASH
    if (flags & FFLAG) {
	sbd_flashinfo (&flashbuf, &flashsize);
	if (flashsize == 0) {
	    printf ("No flash on this target\n");
	    return 0;
	}
	/* any loaded program will be trashed... */
	flags &= ~(SFLAG | BFLAG | EFLAG);
	flags |= NFLAG;		/* don't bother with symbols */

	/*
	 * Override any offset given on command line.
	 * Addresses should be 0 based, so load them into the flash buffer.
	 */
	offset = (unsigned long) flashbuf;
    }
#endif

    dl_initialise (offset, flags);

    if (!hostport)
      hostport = getenv ("hostport");
    fp = fopen (hostport, "r");
    if (!fp) {
	printf ("can't open %s\n", hostport);
	return (1);
    }
    dltype = -1;		/* dltype is set by 1st char of 1st record */

    if (ioctl (fileno (fp), TCGETA, &tbuf) >= 0) {
	istty = 1;

	otbuf = tbuf;

	dlproto = matchenv ("dlproto");
	dlecho = matchenv ("dlecho");
	if (!baudrate)
	    baudrate = getenv ("hostbaud");

	if (baudrate) {
	    const char *err;
	    if (err = parsettymode (baudrate, &tbuf)) {
		printf ("load: %s: %s\n", err, baudrate);
		return (-1);
	    }
	}
	
	tbuf.c_iflag &= ~IXOFF;		/* we do input flow control */
	tbuf.c_lflag &= ~ISIG;		/* signal handling disabled */

	if (dlecho != ON)
	  tbuf.c_lflag &= ~ECHO;	/* no automatic echo */

	/* allow LF or ETX as line terminator */
	tbuf.c_cc[VEOL] = '\n';
	tbuf.c_cc[VEOL2] = ETX;

	printf ("Downloading from %s, ^C to abort\n", hostport);
	ioctl (fileno (fp), TCSETAF, &tbuf);

	if (cmdstr) {
	    fputs (cmdstr, fp);
	    putc ('\r', fp);
	}

#ifdef INET
	/* spot a network device because it doesn't support GETINTR */
	if (ioctl (fileno (fp), GETINTR, &ointr) < 0)
	    isnet = 1;
#endif
    }
    else {
	printf ("Downloading from %s, ^C to abort\n", hostport);
    }

    err = intr = tot = recno = 0;
    bzero (errs, sizeof (errs));

#ifdef INET
    if (istty && !isnet)
	/* block network polling during serial download */
	ospl = splhigh ();
#endif

    if (setjmp (ldjmpb)) {
	intr = 1;
#ifdef INET
	if (istty && !isnet)
	    /* re-enable network polling */
	    splx (ospl);
#endif
	goto done;
    }

#ifdef INET
    ointr = signal (SIGINT, ldintr);
#else
    ioctl (STDIN, GETINTR, &ointr);
    ioctl (STDIN, SETINTR, ldjmpb);
#endif

    for (;;) {
	scandevs ();

	/* read one line */
	if (istty) {
	    n = fread (recbuf, 1, DLREC - 1, fp);
	    if (n < 0) {
		err++;
		errs[DL_IOERR]++;
		break;
	    }
	    if (n == 0)
		continue;
	} else {
	    if (!fgets (recbuf, DLREC - 1, fp))
	      break;
	    n = strlen (recbuf);
	}

	if (dlproto == XONXOFF)
	    putc (XOFF, fp);
	scandevs ();

#ifdef INET
	if (istty && !isnet) {
	    /* now safe to poll network */
	    splx (ospl);
	    scandevs ();
	}
#endif

	++recno;
	for (ep = recbuf + n; ep > recbuf; ep--) 
	  if (!isspace (ep[-1]) && ep[-1] != ETX)
	    break;

	if (ep > recbuf) {
	    *ep = '\0';	/* terminate line */

	    n = ep-recbuf;

	    if (dltype == -1) {
		char c = recbuf[0];
		if (c == '/')
		  dltype = 1;
		else if (c == 'S')
		  dltype = 0;
		else {
		    err++;
		    errs[DL_BADCHAR]++;
		    if (flags & VFLAG) {
			printf ("unknown record type '%c' (0x%x)\n",
				isprint(c) ? c : '?', c & 0xff);
		    }
		    break;
		}
	    }

	    if (dltype == 1)
	      s = dl_fastload (recbuf, n, &len, flags);
	    else if (dltype == 0)
	      s = dl_s3load (recbuf, n, &len, flags);
	    
	    tot += len;
	    if (s == DL_DONE) {
		break;
	    }
	    if (s != DL_CONT) {
		err++;
		errs[s]++;
		if (flags & VFLAG) {
		    printf ("line %d: %s\n", recno, dl_err (s));
		    printf ("%s\n", recbuf);
		}
		if (dlproto == ETXACK)
		    break;
	    }
	}

#ifdef INET
	if (istty && !isnet)
	    /* block network polling again */
	    ospl = splhigh ();
#endif

	if (dlproto == XONXOFF)
	  putc (XON, fp);
	else if (dlproto == ETXACK)
	  putc (ACK, fp);
	if (dlecho == LFEED)
	  putc ('\n', fp);
    }

done:
#ifdef INET
    signal (SIGINT, ointr);
#else
    ioctl (STDIN, SETINTR, ointr);
#endif

    if (dlproto == XONXOFF)
      putc (XON, fp);
    else if (dlproto == ETXACK)
      putc (err ? NAK : ACK, fp);
    if (dlecho == LFEED)
      putc ('\n', fp);
    if (istty)			/* restore tty state */
	ioctl (fileno (fp), TCSETAF, &otbuf);
    fclose (fp);

    if (err != 0 || intr) {
	printf ("\n%d error%s%s\n", err, (err == 1) ? "" : "s", 
		intr ? " (interrupted)" : "");
	for (i = 0; i < DL_MAX; i++)
	    if (errs[i])
		printf ("   %3d %s\n", errs[i], dl_err(i));
	printf ("total = 0x%x bytes\n", tot);
	return (1);
    }

    
    printf ("\ntotal = 0x%x bytes\n", tot);

    if (!(flags & (FFLAG|YFLAG))) {
	printf ("Entry Address  = %08x\n", dl_entry);
	flush_cache (ICACHE);
	flush_cache (DCACHE);
	Epc = dl_entry;
	if (!(flags & SFLAG))
	    dl_setloadsyms ();
    }

#ifdef FLASH
    if (flags & FFLAG) {
	extern long dl_minaddr;
	extern long dl_maxaddr;
	/* default assume same endianness */
#if #endian(big)
	int bigend = 1;
#else
	int bigend = 0;
#endif
	if (flags & WFLAG)
	    /* code endianness is the opposite current prom */
	    bigend = !bigend;
	sbd_flashprogram ((void *)dl_minaddr, 	   	/* address */
			  dl_maxaddr - dl_minaddr, 	/* size */
			  dl_minaddr - (long)flashbuf,  /* offset */
			  bigend);
    }
#endif

    return (0);
}


#endif /* NO_SERIAL */

