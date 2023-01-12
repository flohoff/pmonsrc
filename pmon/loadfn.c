/* $Id */

/*
 * S3 & LSI support functions
 */

#include "mips.h"
#include "pmon.h"
#include "string.h"

#include "loadfn.h"

/*
 * Useful S-records for testing purposes:
 *      S0030000FC
 *      S309900200001234567860
 *      S7058002000073
 *
 * Useful fast-records for testing purposes:
 *      /ACgAgAA        - addr = a0020000
 *      /ZAI            - clear 24 bytes
 *      /E              - define entry at 'addr' and exit
 *      AAABAAAC        - 00 00 01 00 00 02
 *      /Sfred,X        - symbol = 'addr'
 *      /BAB            - byte of 01
 */

#if defined(INET) || !defined(NO_SERIAL)

static const char *dlerrs[] = {
    "continue(!)",		/* DL_CONT */
    "done(!)",			/* DL_DONE */
    "bad char",			/* DL_BADCHAR */
    "bad length",		/* DL_BADLEN */
    "bad type",			/* DL_BADTYPE */
    "bad checksum",		/* DL_BADSUM */
    "out of symbol space",	/* DL_NOSYMSPACE */
    "bad load address",		/* DL_BADADDR */
    "i/o error"			/* DL_IOERR */
};
#define NERRS	(sizeof(dlerrs)/sizeof(dlerrs[0]))

long	dl_entry;	/* entry address */
long	dl_offset;	/* load offset */
long	dl_minaddr;	/* minimum modified address */
long	dl_maxaddr;	/* maximum modified address */

static int	dl_chksum;



char const *
dl_err (int code)
{
    if (code >= DL_MAX)
	return "unknown";
    return dlerrs[code];
}

static inline int
overlap (as, ae, bs, be)
    unsigned long as, ae, bs, be;
{
    return (ae > bs && be > as);
}


int 
dl_checkloadaddr (void *vaddr, int size, int verbose)
{
    extern char _ftext[], etext[], _fdata[], edata[], _fbss[], end[];
    extern char *heaptop;
    unsigned long start, finish;
    unsigned long addr = (unsigned long)vaddr;
    /* check that virtual address is in KSEG0/1 */
    if (addr < K0BASE || addr + size > K2BASE) {
	if (verbose)
	    printf ("\nattempt to load into mappable region");
	return 0;
    }

    /* now check physical addresses */
    start = K0_TO_PHYS (addr);
    finish = start + size;
    if (overlap (start, finish, 0x1fc00000, 0x20000000)) {
	if (verbose)
	    printf ("\nattempt to load into PROM space");
	return 0;
    }
    if (overlap (start, finish, K0_TO_PHYS (_ftext), K0_TO_PHYS (etext))) {
	if (verbose)
	    printf ("\nattempt to overwrite monitor code");
	return 0;
    }
    if (overlap (start, finish, K0_TO_PHYS (_fdata), K0_TO_PHYS (heaptop))) {
	if (verbose)
	    printf ("\nattempt to overwrite monitor data");
	return 0;
    }
    if (overlap (start, finish, memorysize, K0SIZE)) {
	if (verbose)
	    printf ("\nattempt to load outside of memory");
	return 0;
    }

    if (addr < dl_minaddr)
	dl_minaddr = addr;
    if (addr + size > dl_maxaddr)
	dl_maxaddr = addr + size;

    return 1;
}


void
dl_initialise (unsigned long offset, int flags)
{
    if (!(flags & SFLAG))
	clrsyms ();
    if (!(flags & BFLAG))
	clrbpts ();
    if (!(flags & EFLAG))
	clrhndlrs ();
    _mips_watchpoint_reset ();

    dl_minaddr = ~0;
    dl_maxaddr = 0;
    dl_chksum = 0;
    dl_offset = offset;
}

void
dl_setloadsyms (void)
{
    defsyms (dl_minaddr, dl_maxaddr, Epc);
}


/*************************************************************
 *  s3_datarec(p,plen,flags)
 *      handle an S-record data record
 */
static int
s3_datarec (char *p, int *plen, int flags)
{
    int             len, i, cs;
    unsigned long   addr, v;

    *plen = 0;
    dl_chksum = 0;

    if (flags & YFLAG)		/* only load symbols? */
	return DL_CONT;

    if (!gethex (&len, &p[2], 2))
	return (DL_BADCHAR);

    dl_chksum += len;

    if (len * 2 != strlen (p) - 4)
	return (DL_BADLEN);

    i = 4;
    addr = 0;
    switch (p[1]) {
    case '3':
	if (!gethex (&v, &p[i], 2))
	    return (DL_BADCHAR);
	addr = (addr << 8) + v;
	dl_chksum += v;
	i += 2;
    case '2':
	if (!gethex (&v, &p[i], 2))
	    return (DL_BADCHAR);
	addr = (addr << 8) + v;
	dl_chksum += v;
	i += 2;
    case '1':
	if (!gethex (&v, &p[i], 2))
	    return (DL_BADCHAR);
	addr = (addr << 8) + v;
	dl_chksum += v;
	if (!gethex (&v, &p[i + 2], 2))
	    return (-1);
	addr = (addr << 8) + v;
	dl_chksum += v;
	break;
    default:
	return (DL_BADTYPE);
    }

    addr += dl_offset;
    len -= (i / 2) + 1;
    if (!dl_checkloadaddr ((void *)addr, len, flags & VFLAG))
	return (DL_BADADDR);

    p = &p[i + 4];
    for (i = 0; i < len; i++, p += 2) {
	if (!gethex (&v, p, 2))
	    return (DL_BADCHAR);
	store_byte (addr++, v);
	dl_chksum += v;
    }

    if (!gethex (&cs, p, 2))
	return (DL_BADCHAR);
    if (!(flags & IFLAG) && cs != ((~dl_chksum) & 0xff))
	return (DL_BADSUM);

    *plen = i;
    return (DL_CONT);
}

/*************************************************************
 *  s3_dlsym(p, flags)
 *      handle S4 records, i.e., symbols
 */
static int
s3_dlsym (char *p, int flags)
{
    char            val;
    word            adr;
    int             len, csum;
    char            name[LINESZ], *t;

/* 
 * S4LLAAAAAAAANNNNNNNN,AAAAAAAANNNNNN,AAAAAAAANNNNNNNNNN,CC
 * LL=length AAAAAAAA=addr NNNN,=name CC=checksum 
 */

    if (flags & NFLAG)		/* no symbols? */
	return (DL_CONT);

    p += 2;			/* skip past S4 */
    if (!gethex (&len, p, 2))
	return (DL_BADCHAR);
    p += 2;			/* skip past length */
    for (; len > 2;) {
	if (!gethex (&adr, p, 8))
	    return (DL_BADCHAR);
	p += 8;			/* skip past addr */
	len -= 8;

	t = strchr (p, ',');
	if (t == 0)
	    return (DL_BADCHAR);
	strncpy (name, p, t - p);
	name[t - p] = '\0';
	len -= t - p;

	if (!newsym (name, (flags & AFLAG) ? adr : adr + dl_offset))
	    return (DL_NOSYMSPACE);

	p = t + 1;
    }
    if (!gethex (&csum, p, 2))
	return (DL_BADCHAR);

    /* csum neither generated nor checked */
    return (DL_CONT);
}

/*************************************************************
 *  s3_endrec(p)
 *      handle the S-record termination record
 */
static int
s3_endrec (char *p)
{
    word            adr;

    if (gethex (&adr, &p[4], ('9' - p[1] + 2) * 2))
	dl_entry = adr + dl_offset;
    else
	dl_entry = 0xdeadbeef;

    /* always return DL_DONE regardless of record errors */
    return DL_DONE;
}

/*************************************************************
 *  dl_s3load(recbuf,plen,flags)
 *      load Motorola S-records
 */
int
dl_s3load (char *recbuf, int recsize, int *plen, int flags)
{
    *plen = 0;

    if (recbuf[0] != 'S')
	return DL_BADCHAR;

    if (recbuf[1] == '0')
	return (DL_CONT);

    if (recbuf[1] >= '1' && recbuf[1] <= '3')
	return (s3_datarec (recbuf, plen, flags));

    if (recbuf[1] == '4')
	return (s3_dlsym (recbuf, flags));

    if (recbuf[1] >= '7' && recbuf[1] <= '9')
	return (s3_endrec (recbuf));

    return (DL_BADTYPE);
}



#define __ 64

static const unsigned char etob[128] = {
	__,	__,	__,	__,	__,	__,	__,	__,
	__,	__,	__,	__,	__,	__,	__,	__,
	__,	__,	__,	__,	__,	__,	__,	__,
	__,	__,	__,	__,	__,	__,	__,	__,
	__,	__,	__,	__,	__,	__,	__,	__,
	__,	__,	__,	__,	62,	__,	63,	__,
	52,	53,	54,	55,	56,	57,	58,	59,
	60,	61,	__,	__,	__,	__,	__,	__,
	__,	0,	1,	2,	3,	4,	5,	6,
	7,	8,	9,	10,	11,	12,	13,	14,
	15,	16,	17,	18,	19,	20,	21,	22,
	23,	24,	25,	__,	__,	__,	__,	__,
	__,	26,	27,	28,	29,	30,	31,	32,
	33,	34,	35,	36,	37,	38,	39,	40,
	41,	42,	43,	44,	45,	46,	47,	48,
	49,	50,	51,	__,	__,	__,	__,	__
};

#undef __

long   		nextaddr;
int             blkinx = 0;
int             blksz = 0;

/*************************************************************
 *  getb64(vp,p,n)
 *      get a number of base-64 digits
 *      if n < 0 then don't add value to chksum.
 */
static int
getb64 (unsigned long *vp, char *p, int n)
{
    unsigned long   v, b;
    unsigned i;

    v = 0;
    for (i = ((n < 0) ? 0 - n : n); i != 0; i--) {
	b = etob[*p++ & 0x7f];
	if (b == 64) {
	    *vp = 0;
	    return (0);
	}
	v = (v << 6) | b;
    }

    /* update 12 bit checksum */
    switch (n) {
    case 6:
	dl_chksum += (v >> 24) & 0xfff;
    case 4:
	dl_chksum += (v >> 12) & 0xfff;
    case 2:
	dl_chksum += v & 0xfff;
    }

    *vp = v;
    return (1);
}


/*************************************************************
 *  getrec(p,curblk)
 *      get the next 4-byte record (if there's any left)
 */
static inline char *
getrec (curblk)
     char           *curblk;
{
    char *p;
    if (blkinx >= blksz)
	return (0);
    p = &curblk[blkinx];
    blkinx += 4;
    return (p);
}


/*************************************************************
 *  dl_fastload(recbuf,offset,plen,flags)
 *      load a fast-mode record
 */
int
dl_fastload (char *recbuf, int recsize, int *plen, int flags)
{
    char	    *rp;

    int             n, i, j, c, len, s, x;
    char            name[LINESZ];
    unsigned long   bdat, zcnt;
    long   	    addr;

    *plen = 0;

    if (recsize % 4 != 0)
	return (DL_BADLEN);

    blksz = recsize;
    blkinx = 0;

    s = DL_CONT;
    for (len = 0;;) {
	if (!(rp = getrec (recbuf)))
	    break;
	if (rp[0] == '/') {
	    switch (rp[1]) {
	    case 'K':		/* klear checksum (sic) */
		dl_chksum = 0;
		break;
	    case 'C':		/* compare checksum */
		if (!getb64 (&zcnt, &rp[2], -2))
		    return (DL_BADCHAR);
		dl_chksum &= 0xfff;
		if (!(flags & IFLAG) && zcnt != dl_chksum)
		    return (DL_BADSUM);
		dl_chksum = 0;
		break;
	    case 'Z':		/* zero fill */
		if (!getb64 (&zcnt, &rp[2], 2))
		    return (DL_BADCHAR);
		zcnt *= 3;
		if (flags & YFLAG) { /* symbols only? */
		    nextaddr += zcnt;
		    break;
		}
		if (!dl_checkloadaddr ((void *)nextaddr, zcnt, flags & VFLAG))
		    return (DL_BADADDR);
		len += zcnt;
		while (zcnt--)
		    store_byte (nextaddr++, 0);
		break;
	    case 'B':		/* byte */
		if (!getb64 (&bdat, &rp[2], 2))
		    return (DL_BADCHAR);
		if (flags & YFLAG) { /* symbols only? */
		    nextaddr++;
		    break;
		}
		if (!dl_checkloadaddr ((void *)nextaddr, 1, flags & VFLAG))
		    return (DL_BADADDR);
		store_byte (nextaddr++, bdat);
		len++;
		break;
	    case 'A':		/* address */
		if (!getrec (recbuf)) /* get another 4 bytes */
		    return (DL_BADLEN);
		if (!getb64 (&addr, &rp[2], 6))
		    return (DL_BADCHAR);
		if ((flags & TFLAG) && Gpr[R_A3] == 0) {
		    memorysize -= addr;
		    Gpr[R_SP] = clienttos ();
		    nextaddr = memorysize + K0BASE;
		    Gpr[R_A3] = nextaddr;
		} else
		    nextaddr = addr + dl_offset;
		break;
	    case 'E':		/* end */
		dl_entry = nextaddr;
		s = DL_DONE;
		break;
	    case 'S':		/* symbol */
		i = 2; j = 0;
		while (1) {
		    c = rp[i++];
		    /* dl_chksum += c; */
		    if (c == ',')
			break;
		    name[j++] = c;
		    if (i >= 4) {
			if (!(rp = getrec (recbuf)))
			    return (DL_BADLEN);
			i = 0;
		    }
		}
		name[j] = '\0';
		if (flags & NFLAG) /* no symbols? */
		    break;
		if (!newsym (name, nextaddr))
		    return (DL_NOSYMSPACE);
		break;
	    default:
		return (DL_BADTYPE);
	    }
	} else {		/* 24 bits of data */
	    if (!getb64 (&bdat, rp, 4))
		return (DL_BADLEN);
	    if (flags & YFLAG) {	/* symbols only? */
		nextaddr += 3;
		break;
	    }
	    if (!dl_checkloadaddr ((void *)nextaddr, 3, flags & VFLAG))
		return (DL_BADADDR);
	    for (i = 16; i >= 0; i -= 8) {
		store_byte (nextaddr++, bdat >> i);
	    }
	    len += 3;
	}
    }
    *plen = len;
    return (s);
}


#endif
