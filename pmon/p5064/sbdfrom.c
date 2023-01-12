/*
 * P5064/sbdfrom.c: Flash ROM support for P5064
 * Copyright (c) 1997 Algorithmics Ltd.
 */

#ifdef IN_PMON
#include <stddef.h>
#include "pmon.h"
#include "sde-compat.h"
#else
#include <sys/types.h>
#include <mips/cpu.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <kit_impl.h>
#endif
#include <flashdev.h>
#include "sbd.h"
#include "from.h"


#ifdef DBGFROM
#define DBG(fmt, args...) \
	_mon_printf (fmt , ## args);
#else
#define DBG(fmt, args...)
#endif

static int frominfo (flashcookie_t, unsigned int, struct flashinfo *);
static int fromerasedevice (flashcookie_t);
static int fromerasesector (flashcookie_t, unsigned int);
static int fromprogrambytes (flashcookie_t, unsigned int, const void *, 
			     unsigned int, int);
static int fromreadbytes (flashcookie_t, unsigned int, void *, 
			  unsigned int, unsigned int);

static const struct flashdev flashdev = {
    frominfo,
    fromerasedevice,
    fromerasesector,
    fromprogrambytes,
    fromreadbytes
};


enum bootsects {FBNONE, FBTOP, FBBOT};

static struct mandevinfo {
    const char		*name;
    unsigned char	man;
    unsigned char	dev;
    unsigned int	size;
    enum bootsects	boot;
    unsigned int	secsize;
    unsigned short	unlock1_offs;
    unsigned short	unlock2_offs;
} const mandevinfo[] = {
    /* 4Mb = 512KB devices  */
#define UNLOCK_4Mb	UNLOCK1_4Mb_OFFS, UNLOCK2_4Mb_OFFS
    {"Am29F400T",
     MAN_AMD,		0x23,	0x080000, FBTOP, 0x10000, UNLOCK_4Mb},
    {"Am29F400B",
     MAN_AMD, 		0xab,	0x080000, FBBOT, 0x10000, UNLOCK_4Mb},
    {"Fujitsu 29F400-T",
     MAN_FUJITSU,	0x23,	0x080000, FBTOP, 0x10000, UNLOCK_4Mb},
    {"Fujitsu 29F400-B",
     MAN_FUJITSU,	0xab,	0x080000, FBBOT, 0x10000, UNLOCK_4Mb},
    {"Micron MT28LF400T",
     MAN_MICRON,	0x30,	0x080000, FBTOP, 0x10000, UNLOCK_4Mb},
    {"Micron MT28LF400B",
     MAN_MICRON,	0x31,	0x080000, FBBOT, 0x10000, UNLOCK_4Mb},
    {"Intel 28F400-T",
     MAN_INTEL,		0x70,	0x080000, FBTOP, 0x10000, UNLOCK_4Mb},
    {"Intel 28F400-B",
     MAN_INTEL,		0x71,	0x080000, FBBOT, 0x10000, UNLOCK_4Mb},

    /* 8Mb = 1MB devices  */
#define UNLOCK_8Mb	UNLOCK1_8Mb_OFFS, UNLOCK2_8Mb_OFFS
    {"Am29F080",
     MAN_AMD,		0xd5,	0x100000, FBNONE, 0x10000, UNLOCK_8Mb},
    {"Fujitsu 29F080",
     MAN_FUJITSU,	0xd5,	0x100000, FBNONE, 0x10000, UNLOCK_8Mb},

    {0}
};

#if 1
#define PROGRAM_BASE(cp)	((cp)->base - PROGRAM_OFFS)
#define READ_BASE(cp)		((cp)->base)
#else
#define PROGRAM_BASE(cp)	((cp)->base)
#define READ_BASE(cp)		((cp)->base)
#endif

/* possible locations and max size of flash rom in system */
#define NFLASHDEVS 3
static struct flashcookie cookies[NFLASHDEVS] = {
    /* soldered flash or prom socket (depends on link) */
    {BOOTPROM_BASE,	0x100000,	1,	&flashdev},
    /* explicit soldered-down flash */
    {FLASH_BASE, 	0x100000,	1,	&flashdev},
    /* explicit socketed prom/flash */
    {EPROM_BASE, 	0x080000,	1,	&flashdev},
};

#ifndef IN_PMON
unsigned char	_sbd_flash_secbuf[FROMMAXSECSIZE];
#endif

#define SAFEROUTINE(name,protoargs,callargs) \
static int name protoargs \
{ \
    static int (*fn) protoargs; \
    if (!fn) {\
      size_t codesize = (char *)name - (char *)_##name; \
      fn = malloc (codesize); \
      if (!fn) return FLASHDEV_NOMEM; \
      memcpy (fn, _##name, codesize); \
      mips_clean_cache ((vaddr_t)fn, codesize); \
    } \
    return (*fn) callargs; \
}



/*
 * 8-bit non-interleaved interface
 */

static __inline__ void
fromunlock (volatile from_t *base, const struct mandevinfo *md)
{
    base[md->unlock1_offs] = UNLOCK1_DATA;
    base[md->unlock2_offs] = UNLOCK2_DATA;
}


static __inline__ void
fromcmd (volatile from_t *base, const struct mandevinfo *md, from_t cmd)
{
    base[md->unlock1_offs] = UNLOCK1_DATA;
    base[md->unlock2_offs] = UNLOCK2_DATA;
    base[md->unlock1_offs] = cmd;
}


static __inline__ int
frompoll (volatile from_t *fp, from_t expect)
{
    from_t poll;

    while (1) {
	poll = *fp;
	if (((poll ^ expect) & DQPOLL) == 0)
	    break;
	if (poll & DQTIMEEXCEEDED) {
	    poll = *fp;
	    if (((poll ^ expect) & DQPOLL) == 0)
		break;
	    *fp = FROM_RESET;
	    (void) *fp;
	    return FLASHDEV_FATAL;
	}
    }
    return FLASHDEV_OK;
}

/* read manufacture and device id from chip */
static int
_fromautoselect (flashcookie_t cp, const struct mandevinfo *md, 
		 unsigned int offs, unsigned char *man, unsigned char *dev)
{
    volatile from_t *fp = PA_TO_KVA1 (PROGRAM_BASE (cp));
    int prot;

    fromcmd (fp, md, FROM_AUTOSELECT);

    /* get manufacture and device id */
    if (man)
	*man = fp[0];
    if (dev)
	*dev = fp[1];

    /* get sector protect status */
    prot = *(volatile from_t *)((char *)fp + offs + 2) & 1;

    fp[0] = FROM_RESET;
    (void) fp[0];
    return prot;
}


SAFEROUTINE(fromautoselect, 
	    (flashcookie_t cp, const struct mandevinfo *md, 
	     unsigned int offs, unsigned char *man, unsigned char *dev),
	    (cp, md, offs, man, dev))



flashcookie_t
_sbd_flashopen (paddr_t devaddr)
{
    struct flashcookie *cp;

    /* find device addressed by devaddr */
    for (cp = cookies; cp < &cookies[NFLASHDEVS]; cp++)
	if (READ_BASE (cp) <= devaddr 
	    && devaddr < READ_BASE (cp) + cp->size)
	    break;

    if (cp >= &cookies[NFLASHDEVS])
	return (flashcookie_t) 0;

    /* get device specific info if none there yet */
    if (!cp->data) {
	volatile from_t *fp = PA_TO_KVA1 (PROGRAM_BASE (cp));
	const struct mandevinfo *md;
	unsigned char man, dev;

	/* scan device type table */

	for (md = mandevinfo; md->man != 0; md++) {

	    /* read manufacture and device id from chip */
	    (void) fromautoselect (cp, md, 0, &man, &dev);

	    if (man == fp[0] && dev == fp[1])
		/* read back same, assume its r/o eprom */
		continue;

	    if (md->man == man && md->dev == dev)
		break;
	}

	if (md->man == 0)
	    /* unknown device */
	    return (flashcookie_t) 0;

	/* got it */
	cp->data = md;
    }

    return cp;
}



/*
 * Return the starting offset and total size of the sector
 * containing the given offset
 */

static int
findsector (flashcookie_t cp, unsigned int offs, 
	    unsigned int *soffsp, unsigned int *ssizep)
{
    static const int bootsecs[] = {0x4000, 0x2000, 0x2000, 0};
    const struct mandevinfo *md = cp->data;
    unsigned int soffs;
    int ssize;
    int i;

    if (offs >= md->size) {
	/* out of range */
	soffs = ssize = 0;
    }
    else if (md->boot == FBTOP && offs >= md->size - md->secsize) {
	/* in top boot sector */
	soffs = md->size;
	for (i = 0; ssize = bootsecs[i]; i++) {
	    if (ssize > 0) {
		/* decrement sector base by sector size */
		soffs -= ssize;
	    }
	    else {
	    }
	    if (offs > soffs)
		break;
	}
	if (!ssize) {
	    /* last boot sector */
	    ssize = soffs; 
	    soffs = md->size - md->secsize;
	}
    }
    else if (md->boot == FBBOT && offs < md->secsize) {
	/* in bottom boot sector */
	soffs = 0;
	for (i = 0; ssize = bootsecs[i]; i++) {
	    if (offs < soffs + ssize)
		break;
	    soffs += ssize;
	}
	if (!ssize) {
	    /* last boot sector */
	    ssize = md->secsize - soffs;
	}
    }
    else {
	/* in normal sector */
	soffs = offs & ~(md->secsize - 1);
	ssize = md->secsize;
    }

    if (soffsp)
	*soffsp = soffs;
    if (ssizep)
	*ssizep = ssize;

    if (ssize == 0)
	return 1;
    return fromautoselect (cp, md, soffs, 0, 0);
}


static int
frominfo (flashcookie_t cp, unsigned int offs, struct flashinfo *info)
{
    const struct mandevinfo *md = cp->data;

    /* board specific parameters */
    info->base = cp->base;
    info->unit = cp->unit;
    info->mapbase = cp->base;

    /* flash device specific parameters */
    info->name = md->name;
    info->size =  md->size;
    info->maxssize = md->secsize;

    /* get specified sector information */
    info->sprot = findsector (cp, offs, &info->soffs, &info->ssize);

    return FLASHDEV_OK;
}



/*
 * The "erase" functions don't get copied to RAM, since
 * they will only be called by ROM resident code that 
 * knows what it is doing (i.e. not erasing itself)!
 */

static int
_fromerasedevice (flashcookie_t cp)
{
    const struct mandevinfo *md = cp->data;
    volatile from_t *fp = PA_TO_KVA1 (PROGRAM_BASE (cp));

    fromcmd (fp, md, FROM_ERASE);
    fromcmd (fp, md, FROM_ERASECHIP);
    return frompoll (fp, ~0);
}

SAFEROUTINE(fromerasedevice,
	    (flashcookie_t cp),
	    (cp))


static int
_fromerasesector (flashcookie_t cp, unsigned int secoffset)
{
    const struct mandevinfo *md = cp->data;
    volatile from_t *fp = PA_TO_KVA1 (PROGRAM_BASE (cp));
    volatile from_t *sp;

    if (secoffset >= md->size)
	return (FLASHDEV_FAIL);
    sp = (from_t *)((char *)fp + secoffset);
    fromcmd (fp, md, FROM_ERASE);
    fromunlock (fp, md);
    *sp = FROM_ERASESECT;
    return frompoll (sp, ~0);
}

SAFEROUTINE(fromerasesector,
	    (flashcookie_t cp, unsigned int secoffset),
	    (cp, secoffset))


static int
fromreadbytes (flashcookie_t cp, unsigned int offs, 
	       void *mem, unsigned int nb, unsigned int flags)
{
    const struct mandevinfo *md = cp->data;
    unsigned char *dp;
    int swap;

    /* check limits */
    if (offs + nb > md->size)
	return FLASHDEV_FAIL;

    if (IS_KVA0 (cp))
	dp = PA_TO_KVA0 (READ_BASE (cp));
    else
	dp = PA_TO_KVA1 (READ_BASE (cp));

#if #endian(big)
    /* When reading in big-endian mode the byte-packer swaps bytes 
       within words, so we have to swap back again when reading 
       the flash as a byte stream, or little-endian code */
    swap = ((flags & FLASHDEV_READ_STREAM)
	    || !(flags & FLASHDEV_READ_CODE_EB));
#else
    /* In little-endian mode we only need to byte-swap when 
       reading big-endian code. */
    swap = (flags & FLASHDEV_READ_CODE_EB);
#endif
    if (swap) {
	/* undo the effects of the byte-packer */
	from_t *mp = mem;
	for (nb /= sizeof(from_t); nb != 0; nb--)
	    *mp++ = dp[offs++ ^ 3];
    }
    else {
	/* when not swapping we can read fast */
	memcpy (mem, &dp[offs], nb);
    }
    return FLASHDEV_OK;
}



/* We build a vector of these, each representing the data to be
   programmed into one sector. */
struct flashvec {
    unsigned int 	offs; 
    const void *	mem;
    unsigned int 	nb;
    unsigned int	erase;
    int			scratch;
};
	

static int
_reallyprogrambytes (flashcookie_t cp, const struct flashvec *v, int nv,
		    int flags)
{
    const struct mandevinfo *md = cp->data;
    volatile from_t *fp = PA_TO_KVA1 (PROGRAM_BASE (cp));
    volatile from_t *sp;
    int status = FLASHDEV_OK;
    const from_t *mp;
    unsigned int j, o;
    int stat;

    while (nv-- != 0) {

	/* erase any eproms that need erasing */
	if (v->erase) {
	    DBG ("erase sector at %06x ...", i, v->offs);
	    fromcmd (fp, md, FROM_ERASE);
	    fromunlock (fp, md);
	    sp = (from_t *)((char *)fp + v->offs);
	    *sp = FROM_ERASESECT;
	    stat = frompoll (sp, ~0);
	    if (stat != FLASHDEV_OK)
		status = stat;
	}	    

	DBG ("program sector from %06x-%06x ...",
	     v->offs, v->offs + v->nb - 1);

	mp = (from_t *)v->mem; 
	for (j = v->nb/sizeof(from_t), o = v->offs; j != 0; j--, o++, mp++) {
	    if (flags & FLASHDEV_PROG_CODE_EB)
		/* when programming big-endian code into flash
		   we must munge its byte-in-word address, to
		   match how the byte-packer operates when 
		   reading in big-endian mode. */
		sp = (from_t *)((char *)fp + (o ^ 3));
	    else
		/* otherwise program flash with sequential byte stream */
		sp = (from_t *)((char *)fp + o);

	    if (*sp != *mp) {
		fromcmd (fp, md, FROM_PROGRAM);
		*sp = *mp;
		stat = frompoll (sp, *mp);
		if (stat != FLASHDEV_OK)
		    status = stat;
	    }
	}

	DBG ("done\n");

	/* advance to next sector */
	v++;
    }

    if (flags & FLASHDEV_PROG_REBOOT)
	/* if we've overwritten the prom code, we must reboot */
	((volatile void (*)(void))PA_TO_KVA1(BOOTPROM_BASE)) ();

    return status;
}


SAFEROUTINE(reallyprogrambytes,
	    (flashcookie_t cp, const struct flashvec *v, int nv, int flags),
	    (cp, v, nv, flags))


static int
fromprogrambytes (flashcookie_t cp, unsigned int offs, 
		    const void *mem, unsigned int nbytes, 
		    int flags)
{
    const struct mandevinfo *md = cp->data;
    struct flashvec vec[FROMMAXSECTORS];
    struct flashvec *v = vec;
    unsigned int o, nb;
    int nv = 0;
    int ret;

    /* check limits */
    if (offs + nbytes > md->size)
	return FLASHDEV_FAIL;

    if (!mem || nbytes == 0)
	/* flush buffer */
	return FLASHDEV_OK;

    /* construct the flash programming vector */
    for (o = offs, nb = nbytes; nb != 0;) {
	unsigned int soffs, ssize;
	unsigned int n, i;
	const from_t *sp, *mp;
	int prot;

	/* find current sector */
	prot = findsector (cp, o, &soffs, &ssize);
	if (prot)
	    return FLASHDEV_PROTECTED;

	/* how many bytes do we want to program in this sector */
	n = ssize - (o - soffs);
	if (n > nb)
	    n = nb;

	/* add to programming vector */
	v->offs = o;
	v->nb = n;
	v->mem = mem;
	v->scratch = 0;
	v->erase = 0;

	/* decide if sector needs erasing */
#if 0
	if (IS_KVA0 (cp))
	    sp = (from_t *) PA_TO_KVA0 (READ_BASE (cp) + o);
	else
	    sp = (from_t *) PA_TO_KVA1 (READ_BASE (cp) + o);
	mp = mem;
	for (i = n; i != 0 && v->erase != 1; i -= sizeof(from_t)) {
	    if (*mp++ & ~*sp++)
		v->erase |= 1;
	}
#else
	{
	    /* duplicate the programming procedure */
	    volatile from_t *fp = (from_t *) PA_TO_KVA1 (PROGRAM_BASE (cp) + o);
	    mp = mem;
	    for (i = 0; i < n && v->erase != 1; i += sizeof(from_t)) {
		if (flags & FLASHDEV_PROG_CODE_EB)
		    /* when programming big-endian code into flash
		       we must munge its byte-in-word address, to
		       match how the byte-packer operates when 
		       reading in big-endian mode. */
		    sp = (from_t *)((char *)fp + (i ^ 3));
		else
		    /* otherwise program flash with sequential byte stream */
		    sp = (from_t *)((char *)fp + i);
		if (*mp++ & ~*sp)
		    v->erase |= 1;
	    }
	}
#endif

	if (v->erase && n != ssize) {
	    /* partial sector needs erasing - merge flash and data */
	    unsigned char *sbuf;
	    if (!(flags & FLASHDEV_PROG_MERGE))
		return FLASHDEV_PARTIAL;
	    sbuf = malloc (ssize);
	    if (!sbuf)
		return FLASHDEV_NOMEM;
	    /* get sector from flash */
	    fromreadbytes (cp, soffs, sbuf, ssize, flags);
	    /* merge in new data */
	    memcpy (&sbuf[o - soffs], mem, n);
	    /* point vector at whole sector */
	    v->offs = soffs;
	    v->nb = ssize;
	    v->mem = sbuf;
	    v->scratch = 1;
	}

	/* advance to next sector */
	v++; nv++;
	mem += n;
	o += n;
	nb -= n;
    }

    /* wipe this area of the flash out of the data cache */
    if (IS_KVA0 (cp))
	mips_clean_dcache ((vaddr_t) PA_TO_KVA0 (READ_BASE (cp) + offs), 
			   nbytes);
    
    /* now really do the programming operation */
    ret = reallyprogrambytes (cp, vec, nv, flags);

    for (v = vec; nv != 0; v++, nv--)
	if (v->scratch)
	    free ((char *)v->mem);

    return ret;
}
