/* $Id: commands.c,v 1.11 2000/03/28 00:20:52 nigel Exp $ */
#include "mips.h"
#include "pmon.h"
#include "stdio.h"
#include "termio.h"
#include "set.h"
#include "string.h"

char           *searching = "searching..  ";

char           *badhexsym = "%s: neither hex value nor symbol\n";

/** transp(ac,av), the 'tr' command */
transp (ac, av)
     int             ac;
     char           *av[];
{
    int             fd, n, i;
    jmp_buf         intrsave;
    char           *hostport, buf[80], abortch, *trabort;
    struct termio   tbuf, consave, hostsave;

    trabort = getenv ("trabort");
    abortch = str2cc (trabort);
    if (abortch == 0) {
	printf ("tr: error: trabort not set\n");
	return 1;
    }
    hostport = getenv ("hostport");
    if (strequ (hostport, "tty0")) {
	printf ("can't use tty0 as hostport in transparent mode\n");
	return 1;
    }
    fd = open (hostport, 0);
    if (fd == -1) {
	printf ("can't open %s\n", hostport);
	return 1;
    }
    printf ("Entering transparent mode, %s to abort\n", trabort);

    ioctl (fd, TCGETA, &tbuf);
    hostsave = tbuf;
    tbuf.c_lflag &= ~(ICANON | ECHO | ECHOE);
    tbuf.c_iflag &= ~(ICRNL);
    tbuf.c_iflag |= IXOFF;	/* enable tandem mode */
    tbuf.c_oflag &= ~ONLCR;
    tbuf.c_cc[4] = 1;
    ioctl (fd, TCSETAF, &tbuf);

    ioctl (STDIN, TCGETA, &tbuf);
    consave = tbuf;
    tbuf.c_lflag &= ~(ICANON | ECHO | ECHOE);
    tbuf.c_iflag &= ~(ICRNL | IXON);
    tbuf.c_oflag &= ~ONLCR;
    tbuf.c_cc[4] = 1;
    ioctl (STDIN, TCSETAF, &tbuf);

/* disable INTR char */
    ioctl (STDIN, GETINTR, intrsave);
    ioctl (STDIN, SETINTR, 0);

    for (;;) {
	ioctl (STDIN, FIONREAD, &n);
	if (n > 0) {
	    if (n > sizeof(buf) - 1)
		n = sizeof(buf) - 1;
	    n = read (STDIN, buf, n);
	    buf[n] = '\0';
	    if (strchr (buf, abortch))
		break;
	    write (fd, buf, n);
	}
	ioctl (fd, FIONREAD, &n);
	if (n > 0) {
	    if (n > sizeof(buf))
		n = sizeof(buf);
	    n = read (fd, buf, n);
	    write (STDOUT, buf, n);
	}
    }
    ioctl (STDIN, TCSETAF, &consave);
    ioctl (fd, TCSETAF, &hostsave);
    ioctl (STDIN, SETINTR, intrsave);
    return 0;
}

const Optdesc         m_opts[] =
{
    {"-b", "access bytes"},
    {"-h", "access half-words"},
    {"-w", "access words"},
#ifdef R4000
    {"-d", "access double-words"},
#endif
    {"-n", "non-interactive (no write)"},
    {"", "Interactive Options"},
    {"<hexval>", "set memory, forward one"},
    {"CR", "forward one, no change"},
    {"=", "re-read"},
    {"^|-", "back one"},
    {".", "quit"},
    {0}};

/** modify(ac,av), the 'm' command */
modify (ac, av)
     int             ac;
     char           *av[];
{
    word            adr, i;
    ureg_t	    v;
    char           *p;
    int		    datasz = -2;
    int		    nowrite = 0;
    extern int      optind;
    extern char    *optarg;
    int 	    c;

    optind = 0;
    while ((c = getopt (ac, av, "dwhbn")) != EOF) {
	if (datasz != -2) {
	    printf ("multiple data types specified\n");
	    return (-1);
	}
	switch (c) {
#ifdef R4000
	case 'd':
	    datasz = 8;
	    break;
#endif
	case 'w':
	    datasz = 4;
	    break;
	case 'h':
	    datasz = 2;
	    break;
	case 'b':
	    datasz = 1;
	    break;
	case 'n':
	    nowrite = 1;
	    break;
	default:
	    return (-1);
	}
    }

    if (optind >= ac)
      return (-1);

    if (!get_rsa (&adr, av[optind++]))
      return (-1);

    if (datasz == -2)
      datasz = 1 << matchenv ("datasz");

    if (optind < ac) {
	/* command mode */
	for (; optind < ac; optind++) {
	    if (strequ (av[optind], "-s")) {
		if (++optind >= ac) {
		    printf ("bad arg count\n");
		    return (-1);
		}
		for (p = av[optind]; *p; p++)
		  store_byte (adr++, *p);
	    } else {
		if (!get_rsa_reg (&v, av[optind]))
		    return (-1);
		if (adr & (datasz - 1)) {
		    printf ("%08x: unaligned address\n", adr);
		    return (1);
		}
		switch (datasz) {
		case 1:
		    store_byte (adr, v);
		    break;
		case 2:
		    store_half (adr, v);
		    break;
		case 4:
		    store_word (adr, v);
		    break;
#ifdef R4000
		case 8:
		    store_dword (adr, v);
		    break;
#endif
		}
		adr += datasz;
	    }
	}
    } else {
	/* interactive mode */
	if (adr & (datasz - 1)) {
	    printf ("%08x: unaligned address\n", adr);
	    return (1);
	}
	for (;;) {
	    switch (datasz) {
	    case 1:
		v = *(unsigned char *)adr;
		break;
	    case 2:
		v = *(unsigned short *)adr;
		break;
	    case 4:
		v = *(unsigned int *)adr;
		break;
#ifdef R4000
	    case 8:
		v = *(unsigned long long *)adr;
		break;
#endif
	    }
#if __mips >= 3
	    printf ("%08x %0*llx ", adr, 2 * datasz, v);
#else
	    printf ("%08x %0*lx ", adr, 2 * datasz, v);
#endif
	    if (nowrite) {
		printf ("\n");
		break;
	    }
	    line[0] = '\0'; get_line (line, 0);
	    for (p = line; *p == ' '; p++);
	    if (*p == '.')
	      break;
	    else if (*p == '\0')
	      adr += datasz;
	    else if (*p == '^' || *p == '-')
	      adr -= datasz;
	    else if (*p == '=')	
	      /* reread */;
	    else if (get_rsa_reg (&v, p)) {
		switch (datasz) {
		case 1:
		    store_byte (adr, v);
		    break;
		case 2:
		    store_half (adr, v);
		    break;
		case 4:
		    store_word (adr, v);
		    break;
#ifdef R4000
		case 8:
		    store_dword (adr, v);
		    break;
#endif
		}
		adr += datasz;
	    }
	}
    }
    return (0);
}


/** search(ac,av), the search command */
search (ac, av)
     int             ac;
     char           *av[];
{
    uword           from, to, adr, i, a;
    char           *s, *d, c;
    char            pat[PATSZ];
    int             siz, ln;

    ln = siz = moresz;
    ioctl (STDIN, CBREAK, NULL);

    if (!get_rsa (&from, av[1]) || !get_rsa (&to, av[2]))
	return (-1);
    for (d = pat, i = 3; i < ac; i++) {
	if (strequ (av[i], "-s")) {
	    if (++i >= ac) {
		printf ("bad arg count\n");
		return (-1);
	    }
	    for (s = av[i]; *s; s++)
	      *d++ = *s;
	} else {
	    if (!get_rsa (&a, av[i]))
		return (-1);
	    c = a;
	    *d++ = c;
	}
    }

    if (to <= from) {
	printf ("'to' address too small\n");
	return (1);
    }
    printf ("%s", searching);
    if (from <= to) {		/* forward search */
	to -= d - pat - 1;
	while (from <= to) {
	    s = pat;
	    adr = from++;
	    while (s < d) {
		if (*s != load_byte (adr))
		    break;
		s++;
		adr++;
	    }
	    if (d <= s) {
		era_line (searching);
		dispmem (prnbuf, from - 1, 4);
		if (more (prnbuf, &ln, siz))
		    break;
		printf ("%s", searching);
	    } else
		dotik (1, 0);
	}
	if (from > to)
	    era_line (searching);
    } else {			/* backward search */
	from -= d - pat - 1;
	while (to <= from) {
	    s = pat;
	    adr = from--;
	    while (s < d) {
		if (*s != load_byte (adr))
		    break;
		s++;
		adr++;
	    }
	    if (d <= s) {
		era_line (searching);
		dispmem (prnbuf, from - 1, 4);
		if (more (prnbuf, &ln, siz))
		    break;
		printf ("%s", searching);
	    }
	}
	if (to > from)
	    era_line (searching);
    }
    return (0);
}

/** call(ac,av), the call command */
call (ac, av)
     int             ac;
     char           *av[];
{
    int             i, j, k;
    char           *arg[10];

    arg[0] = 0;
    k = 0;
    for (i = 1; i < ac; i++) {
	if (av[i][0] == '-') {
	    j = 1;
	    while (av[i][j] != 0) {
		if (av[i][j] == 's') {
		    if (++i >= ac) {
			printf ("bad arg count\n");
			return (-1);
		    }
		    arg[k++] = av[i];
		    break;
		} else {
		    printf ("%c: unknown option\n", av[i][j]);
		    return (-1);
		}
		j++;
	    }
	} else {
	    if (!get_rsa ((uword *)&arg[k], av[i]))
		return (-1);
	    k++;
	}
    }
    if (arg[0] == 0) {
	printf ("Function address not specified\n");
	return (-1);
    }
    i = (((Func *) arg[0])) (arg[1], arg[2], arg[3], arg[4], arg[5]);
    printf ("Function returns: 0x%x (%d)\n", i, i);
    return (0);
}


#ifdef E2PROM
e2program (ac, av)
     int             ac;
     char           *av[];
{
    uword	addr, size, offset;

    if (!get_rsa (&addr, av[1])) {
	printf ("bad address\n");
	return -1;
    }

    if (!get_rsa (&size, av[2])) {
	printf ("bad size\n");
	return -1;
    }

    if (!get_rsa (&offset, av[3])) {
	printf ("bad offset\n");
	return -1;
    }

    printf ("Address = %x, Size = %x, Offset = %x\n",
	    addr, size, offset);

    sbd_e2program (addr, size, offset);
    return (0);
}
#endif


#ifdef FLASH
flashprogram (ac, av)
     int             ac;
     char           *av[];
{
    uword	addr, size, offset;
#if #endian(big)
    int		bigend = 1;
#else
    int		bigend = 0;
#endif

    void *fp; int fsize;
    sbd_flashinfo (&fp, &fsize);

    if (fsize == 0) {
	printf ("No flash found\n");
	return (1);
    }

    if (striequ (av[1], "-eb")) {
	if (ac < 5)
	    return (1);
	bigend = 1;
	av++;
    }
    else if (striequ (av[1], "-el")) {
	if (ac < 5)
	    return (1);
	bigend = 0;
	av++;
    }
    else if (striequ (av[1], "-w")) {
	if (ac < 5)
	    return (1);
	bigend = !bigend;
	av++;
    }
	
    if (!get_rsa (&addr, av[1])) {
	printf ("bad address\n");
	return -1;
    }

    if (!get_rsa (&size, av[2])) {
	printf ("bad size\n");
	return -1;
    }

    if (!get_rsa (&offset, av[3])) {
	printf ("bad offset\n");
	return -1;
    }

    printf ("Address = %x, Size = %x, Offset = %x\n",
	    addr, size, offset);

    sbd_flashprogram (addr, size, offset, bigend);
    return (0);
}
#endif


flush_cache (type, adr)
     int             type;
     word            adr;
{

    switch (type) {
    case ICACHE:
#if defined(R3000) || defined(SABLE)
	r3k_iflush ();
#elif defined(LR33000)
	r33k_iflush ();
#elif defined(R4000)
	mips_flush_cache ();	/* flush (and writeback) everything! */
#endif
	break;

    case DCACHE:
#if defined(R3000) || defined(SABLE)
	r3k_dflush ();
#elif defined(LR33000)
	r33k_dflush ();
#elif defined(R4000)
	mips_flush_dcache ();
#endif
	break;

    case IADDR:
#if defined(R3000) || defined(SABLE)
	r3k_iaflush (adr);
#elif defined(LR33000)
	r33k_iaflush (adr);
#elif defined(R4000)
	mips_clean_cache (adr, 1);
#endif
	break;
    }
}

const Optdesc         flush_opts[] =
{
    {"-i", "flush I-cache"},
    {"-d", "flush D-cache"},
    {0}};

/** flush(ac,av), the 'flush' command */
flush (ac, av)
     int             ac;
     char           *av[];
{
    extern int      optind;
    extern char    *optarg;
    int 	    c;
    int             flags = 0;

    optind = 0;
    while ((c = getopt (ac, av, "id")) != EOF) 
      switch (c) {
      case 'd':
	  flags |= 1;
	  break;
      case 'i':
	  flags |= 2;
	  break;
      default:
	  return (-1);
      }

    if (!flags)
	flags = 3;

    if (flags & 2)
	flush_cache (ICACHE);

    if (flags & 1)
	flush_cache (DCACHE);

    return (0);
}

void
store_dword (adr, v)
    word            adr;
    dword	    v;
{
    *(dword *) adr = v;
    flush_cache (IADDR, adr);
}

void
store_word (adr, v)
     word            adr;
     word	     v;
{
    *(word *) adr = v;
    flush_cache (IADDR, adr);
}

void
store_half (adr, v)
     word            adr;
     half            v;
{
    *(half *) adr = v;
    flush_cache (IADDR, adr);
}

void
store_byte (adr, v)
     word            adr;
     byte            v;
{
    *(byte *)adr = v;
    flush_cache (IADDR, adr);
}

#ifndef NO_SERIAL
const Optdesc         dump_opts[] =
{
    {"-B", "dump binary image"},
    {"-h<port>", "send dump to host <port>"},
    {0}};

/** sdump(ac,av), the 'dump' command */
sdump (ac, av)
     int             ac;
     char           *av[];
{
    uword           adr, siz, len, i, a;
    char            *tmp;
    char           *uleof, *ulcr, *hostport = 0, *eol;
    int             fd, cs, v, binary = 0;
    struct termio   tbuf;
    extern int      optind;
    extern char    *optarg;
    int 	    c;

    optind = 0;
    while ((c = getopt (ac, av, "Bh:")) != EOF) 
      switch (c) {
      case 'B':
	  binary = 1;
	  break;
      case 'h':
	  hostport = optarg;
	  break;
      default:
	  return (-1);
      }

    if (optind + 2 > ac)
      return (-1);
    if (!get_rsa (&adr, av[optind++]))
      return (-1);
    if (!get_rsa (&siz, av[optind++]))
      return (-1);

    if (!hostport)
      hostport = (optind < ac) ? av[optind++] : getenv ("hostport");

    if (optind < ac)
      return (-1);

    fd = open (hostport, 1);
    if (fd == -1) {
	printf ("can't open %s\n", hostport);
	return (1);
    }

    if (binary) {
	if (ioctl (fd, TCGETA, &tbuf) >= 0) {
	    printf ("can't dump binary to tty\n");
	    return (1);
	}
	write (fd, adr, siz);
    } else {
	ioctl (fd, TCGETA, &tbuf);
	tbuf.c_iflag &= ~IXANY;
	tbuf.c_oflag &= ~ONLCR;
	ioctl (fd, TCSETAF, &tbuf);
	
	uleof = getenv ("uleof");
	ulcr = getenv ("ulcr");
	if (striequ (ulcr, "cr"))
	  eol = "\r";
	else if (striequ (ulcr, "lf"))
	  eol = "\n";
	else /* crlf */
	  eol = "\r\n";
	
	while (siz > 0) {
	    if (siz < 32)
	      len = siz;
	    else
	      len = 32;
	    cs = len + 5;
	    for (i = 0; i < 4; i++)
	      cs += (adr >> (i * 8)) & 0xff;
	    sprintf (line, "S3%02X%08X", len + 5, adr);
	    for (a = adr, tmp = line + 12, i = 0; i < len; a++, i++) {
		v = load_byte (a);
		cs += v;
		sprintf (tmp, "%02X", v & 0xff);
		tmp += 2;
	    }
	    sprintf (tmp, "%02X%s", (~cs) & 0xff, eol);
	    tmp += 2 + strlen (eol);
	    write (fd, line, tmp - line);
	    adr += len;
	    siz -= len;
	}
	sprintf (line, "S70500000000FA%s", eol);
	write (fd, line, strlen (line));
	write (fd, uleof, strlen (uleof));
    }

    close (fd);
    return (0);
}
#endif

/** copy(ac,av), the 'copy' command */
copy (ac, av)
     int             ac;
     char           *av[];
{
    uword           from, to, n;

    if (!get_rsa (&from, av[1]) || !get_rsa (&to, av[2]) || !get_rsa (&n, av[3]))
	return (-1);
    if (to < from)
	while (n-- > 0)
	    store_byte (to++, load_byte (from++));
    else
	for (from += n, to += n; n-- > 0;)
	    store_byte (--to, load_byte (--from));
    return (0);
}

const Optdesc         d_opts[] =
{
    {"-b", "display as bytes"},
    {"-h", "display as half-words"},
    {"-w", "display as words"},
#ifdef R4000
    {"-d", "display as double-words"},
#endif
    {"-s", "display a null terminated string"},
    {"-r<reg>", "display as register"},
    {0}};

/** dump(ac,av), the 'd' command */
dump (ac, av)
     int             ac;
     char           *av[];
{
    word            adr, x;
    char           *reg;
    int             siz, ln, i;
    int		    datasz = -2;
    extern int      optind;
    extern char    *optarg;
    int 	    c;
    static word     last_adr;

    optind = 0;
    while ((c = getopt (ac, av, "dwhbsr:")) != EOF) {
	if (datasz != -2) {
	    printf ("multiple data types specified\n");
	    return (-1);
	}
	switch (c) {
#ifdef R4000
	case 'd':
	    datasz = 8;
	    break;
#endif
	case 'w':
	    datasz = 4;
	    break;
	case 'h':
	    datasz = 2;
	    break;
	case 'b':
	    datasz = 1;
	    break;
	case 's':
	    datasz = 0;
	    break;
	case 'r':
	    reg = optarg;
	    datasz = -1;
	    break;
	default:
	    return (-1);
	}
    }

    /* get <addr> */
    if (optind >= ac || !get_rsa (&adr, av[optind++]))
      return (-1);

    if( repeating_cmd )
      adr = last_adr;
    
    /* get [<size>|<reg>] */
    if (optind < ac) {
	if (get_rsa (&x, av[optind])) {
	    /* size */
	    optind++;
	    ln = x; siz = 0;
	} else if (getregadr (0, av[optind])) {
	    if (datasz != -2) {
		printf ("multiple data types specified\n");
		return (-1);
	    }
	    reg = av[optind++];
	    datasz = -1;
	} else
	  return (-1);
    } else {
	ln = siz = moresz;
    }

    if (optind != ac) 
      return (-1);

    if (datasz == 0) {		/* -s, print string */
	strncpy (prnbuf, adr, 70);
	prnbuf[70] = '\0';
	for (i = 0; prnbuf[i] != 0; i++)
	    if (!isprint (prnbuf[i]))
		prnbuf[i] = '.';
	printf ("%s\n", prnbuf);
	return (0);
    }

    ioctl (STDIN, CBREAK, NULL);
    if (datasz == -1) {
	/* -r<reg> print as register */
	if (!disp_as_reg ((sreg_t *)adr, reg, &ln)) {
	    printf ("%s: bad register name\n", reg);
	    return (-1);
	}
    } else {
	if (datasz < 0)
	  datasz = 1 << matchenv ("datasz");
	while (1) {
	    last_adr = adr;
	    adr = dispmem (prnbuf, adr, datasz);
	    if (more (prnbuf, &ln, siz))
	      break;
	}
    }

    return (0);
}

word 
dispmem (p, adr, siz)
     char           *p;
     word            adr;
     int             siz;
{
    int             i;
    char            v;
    char            tmp[18];
    union {
	double		dummy;	/* ensure 8-byte alignment */
	unsigned char   b[16];
    } buf;
    word            w;

    w = adr;
    for (i = 0; i < 16; i++) {
	v = load_byte (w++);
	buf.b[i] = v;
    }

    sprintf (p, "%08x  ", adr);
    for (i = 0; i < 16; i += siz) {
	if (i == 8)
	    strccat (p, ' ');
	switch (siz) {
	case 1:
	    sprintf (tmp, "%02x ", *(unsigned char *)&buf.b[i]);
	    break;
	case 2:
	    sprintf (tmp, "%04x ", *(unsigned short *)&buf.b[i]);
	    break;
	case 4:
	    sprintf (tmp, "%08x ", *(unsigned long *)&buf.b[i]);
	    break;
#ifdef R4000
	case 8:
	    sprintf (tmp, "%016llx ", *(unsigned long long *)&buf.b[i]);
	    break;
#endif
	}
	strcat (p, tmp);
    }
    strcat (p, "  ");
    for (i = 0; i < 16; i++) {
	v = buf.b[i];
	strccat (p, isprint (v) ? v : '.');
    }
    return (adr + 16);
}

/** fill(ac,av) the fill command */
fill (ac, av)
     int             ac;
     char           *av[];
{
    uword           from, to, i, a, w;
    ubyte	    *p, *d;
    union {
	uword	w;
	uhalf	h;
	ubyte	b[PATSZ];
    } pat;
    int             len;

    if (!get_rsa (&from, av[1]) || !get_rsa (&to, av[2]))
	return (-1);
    if (to < from)
	return (1);

    for (d = p = pat.b, i = 3; i < ac; i++) {
	if (strequ (av[i], "-s")) {
	    if (++i >= ac) {
		printf ("bad arg count\n");
		return (-1);
	    }
	    else {
		char *s;
		for (s = av[i]; *s; s++) {
		    if (d >= &pat.b[PATSZ]) {
			printf ("pattern too long\n");
			return (-1);
		    }
		    *d++ = *s;
		}
	    }
	} else {
	    if (!get_rsa (&a, av[i]))
		return (-1);
	    if (d >= &pat.b[PATSZ]) {
		printf ("pattern too long\n");
		return (-1);
	    }
	    *d++ = a;
	}
    }

    len = d - p;
    if ((len == 1 || len == 2 || len == 4)
	&& (from & 3) == 0 && (to & 3) == 3) {
	/* special case using word writes */
	switch (len) {
	case 1:
	    w = pat.b[0];
	    w |= (w << 8);
	    w |= (w << 16);
	    break;
	case 2:
	    w = pat.h;
	    w |= (w << 16);
	    break;
	case 4:
	    w = pat.w;
	    break;
	}
	for (; from <= to; from += sizeof (uword))
	    *(uword *)from = w;
    } else {
	/* all other cases: byte by byte */
	for (; from <= to; from += sizeof (ubyte)) {
	    *(ubyte *)from = *p;
	    if (++p >= d)
	      p = pat.b;
	}
    }

    flush_cache (ICACHE, 0);
    return (0);
}

get_rsa (vp, p)
     uword	    *vp;
     char	    *p;
{
    ureg_t	    val;
    
    if (get_rsa_reg (&val, p)) {
	*vp = (uword) val;
	return (1);
    }
    return (0);
}


get_rsa_reg (vp, p)
     ureg_t	    *vp;
     char           *p;
{
    int             r, inbase, inalpha;
    word	    adr;
    ureg_t   	    v1, v2;
    char           *q, subexpr[LINESZ];

/* strip enclosing parens */
    while (*p == '(' && strbalp (p) == p + strlen (p) - 1) {
	strdchr (p);
	p[strlen (p) - 1] = 0;
    }

    if (q = strrpset (p, "+-")) {	/* is compound */
	strncpy (subexpr, p, q - p);
	subexpr[q - p] = '\0';
	r = get_rsa_reg (&v1, subexpr);
	if (r == 0)
	    return (r);
	r = get_rsa_reg (&v2, q + 1);
	if (r == 0)
	    return (r);
	if (*q == '+')
	    *vp = v1 + v2;
	else
	    *vp = v1 - v2;
	return (1);
    }
    if (q = strrpset (p, "*/")) {
	strncpy (subexpr, p, q - p);
	subexpr[q - p] = '\0';
	r = get_rsa_reg (&v1, subexpr);
	if (r == 0)
	    return (r);
	r = get_rsa_reg (&v2, q + 1);
	if (r == 0)
	    return (r);
	if (*q == '*')
	    *vp = v1 * v2;
	else {
	    if (v2 == 0) {
		printf ("divide by zero\n");
		return (0);
	    }
	    *vp = v1 / v2;
	}
	return (1);
    }
    if (*p == '^') {
	r = get_rsa_reg (&v2, &p[1]);
	if (r == 0)
	    printf ("%s: bad indirect address\n", &p[1]);
	else
	  *vp = load_word ((word)v2);
    }
    else if (*p == '@') {
	r = getreg (vp, &p[1]);
	if (r == 0)
	    printf ("%s: bad register name\n", &p[1]);
    } else if (strequ (p, ".")) {
	r = getreg (vp, "epc");
    } else if (*p == '&') {
	r = sym2adr (&adr, &p[1]);
	if (r == 0)
	    printf ("%s: bad symbol name\n", &p[1]);
	else
	    *vp = adr;
    } else if (isdigit (*p)) {
	inbase = matchenv ("inbase");
	switch (inbase) {
	case TEN:
	    r = ator (vp, p, 10);
	    break;
	case SIXTEEN:
	    r = ator (vp, p, 16);
	    break;
	case EIGHT:
	    r = ator (vp, p, 8);
	    break;
	case AUTO:
	    r = ator (vp, p, 0);
	    break;
	default:
	    printf ("%s: bad inbase value\n", getenv ("inbase"));
	    return (0);
	}
	if (r == 0) {
	    r = ator (vp, p, 0);
	    if (r == 0)
		printf ("%s: bad base %s value\n",
			p, getenv ("inbase"));
	}
    } else if (isxdigit (*p)) {
	inalpha = matchenv ("inalpha");
	if (inalpha == HEX) {
	    r = ator (vp, p, 16);
	    if (r == 0) {
		r = sym2adr (&adr, p);
		if (r == 0)
		    printf (badhexsym, p);
		else
		    *vp = adr;
	    }
	} else if (inalpha == SYMBOL) {
	    r = sym2adr (&adr, p);
	    if (r == 0) {
		r = ator (vp, p, 16);
		if (r == 0)
		    printf (badhexsym, p);
		else
		    *vp = adr;
	    }
	} else {
	    printf ("%s: bad inalpha value\n", getenv ("inalpha"));
	    return (0);
	}
    } else {
	r = sym2adr (&adr, p);
	if (r == 0)
	    printf ("%s: bad symbol name\n", p);
	else
	  *vp = adr;
    }
    return (r);
}

reboot_cmd (ac, av)
    int             ac;
    char           *av[];
{
    volatile void (*rebootaddr)() = (void (*))0xbfc00000;

    printf ("Rebooting...\n");

    (*rebootaddr)();
}

#ifdef SROM
const Optdesc         srom_opts[] =
{
    {"-n", "don't execute softROM code"},
    {0}};

sbd_srom (ac, av)
    int             ac;
    char           *av[];
{
    uword           from, to, n;
    int		    dogo, docopy;
    extern int      optind;
    int		    c;

    dogo = 1;
    docopy = 0;

    optind = 0;
    while ((c = getopt (ac, av, "n")) != EOF) {
	switch (c) {
	case 'n':
	    dogo = 0;
	    break;
	default:
	    return (-1);
	}
    }

    from = to = 0;
    if (optind+3 <= ac) {
	if (!get_rsa (&from, av[optind++]) ||
	    !get_rsa (&to, av[optind++]) ||
	    !get_rsa (&n, av[optind++]))
	    return (-1);
    }
    if (optind != ac)
	return (-1);

    /* copy softrom code */
    sbd_softromcopy (from, to, n);
    if (dogo)
	sbd_softromgo ();
    return (0);
}
#endif
