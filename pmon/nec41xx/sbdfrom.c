/*
 * NEC41XX/sbdfrom.c: 32-bit flash ROM support for NEC Vr41xx eval board
 * Copyright (c) 1998 Algorithmics Ltd.
 */

#ifdef IN_PMON
#include "pmon.h"
#include "sde-compat.h"
#include <assert.h>
#else
#include <sys/types.h>
#include <mips/cpu.h>
#include <stdlib.h>
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


static struct mandevinfo {
    const char		*name;
    unsigned char	man;
    unsigned short	dev;
    unsigned int	size;
} const mandevinfo[] = {
    /* 8Mb = 4 x 2MB devices  */
    {"Sharp LH28F032",
     MAN_SHARP,		0x6688,	0x200000*4},
    {0}
};


/* We always program in multiples of an 8-byte line, since this is
   much faster (we can program 4 devices in parallel */
#define LINESIZE	8
#define LINEMASK	(LINESIZE-1)

/* possible locations and max size of flash rom in system */
#define NFLASHDEVS 2
static struct flashcookie cookies[NFLASHDEVS] = {
    {FLASH_BASE,		0x200000*4,	LINESIZE,	&flashdev},
    {FLASH_BASE+0x800000,	0x200000*4,	LINESIZE,	&flashdev},
};


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
 * 32-bit interface, 2-way interleaved 16-bit wide flash 
 * therefore each device is mapped at 4 bytes intervals.
 */

static __inline__ void
fromunlock (volatile from_t *base, const struct mandevinfo *md)
{
    base[UNLOCK1_OFFS*4] = UNLOCK1_DATA;
    base[UNLOCK2_OFFS*4] = UNLOCK2_DATA;
}


static __inline__ void
fromcmd (volatile from_t *base, const struct mandevinfo *md, from_t cmd)
{
    base[UNLOCK1_OFFS*4] = UNLOCK1_DATA;
    base[UNLOCK2_OFFS*4] = UNLOCK2_DATA;
    base[UNLOCK1_OFFS*4] = cmd;
}


static __inline__ void
fromreset (volatile from_t *base)
{
    base[0] = FROM_RESET;
    (void) base[0];	/* uncached read to force write buffer flush */
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
	    fromreset (fp);
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
    volatile from_t *fp = PA_TO_KVA1 (cp->base);
    volatile from_t *sp;
    int prot;

    fromcmd (fp, md, FROM_AUTOSELECT);

    if (man)
	*man = fp[0*4];

    if (dev)
	*dev = fp[1*4];

    /* get sector protect status of (one) device */
    sp = (volatile from_t *)((char *)fp + offs);
    prot = sp[2*4] & 1;

    fromreset (fp);

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
	if (cp->base <= devaddr && devaddr < cp->base + cp->size)
	    break;

    if (cp >= &cookies[NFLASHDEVS])
	return (flashcookie_t) 0;

    /* get device specific info if none there yet */
    if (!cp->data) {
	volatile from_t *fp = PA_TO_KVA1 (cp->base);
	const struct mandevinfo *md;
	unsigned char man, dev;

	/* scan device type table (assume all 4 devices are the same) */

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
    const struct mandevinfo *md = cp->data;
    unsigned int soffs;
    int ssize;
    int i;

    if (offs >= md->size) {
	/* out of range */
	soffs = ssize = 0;
    } else {
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



static int
_fromerasedevice (flashcookie_t cp)
{
    const struct mandevinfo *md = cp->data;
    volatile from_t *fp = PA_TO_KVA1 (cp->base);
    int status = FLASHDEV_OK;
    int i;

    /* erase all in parallel */
    for (i = 0; i < 4; i++) {
	fromcmd (fp+i, md, FROM_ERASE);
	fromcmd (fp+i, md, FROM_ERASECHIP);
    }

    /* wait for all to finish */
    for (i = 0; i < 4; i++) {
	int stat = frompoll (fp+i, ~0);
	if (stat != FLASHDEV_OK)
	    status = stat;
    }

    return status;
}

SAFEROUTINE(fromerasedevice,
	    (flashcookie_t cp),
	    (cp))


static int
_fromerasesector (flashcookie_t cp, unsigned int secoffset)
{
    const struct mandevinfo *md = cp->data;
    volatile from_t *fp = PA_TO_KVA1 (cp->base);
    volatile from_t *sp;
    int status = FLASHDEV_OK;
    int i;

    if (secoffset >= md->size)
	return FLASHDEV_FAIL;

    sp = (from_t *)((char *)fp + secoffset);

    /* erase all in parallel */
    for (i = 0; i < 4; i++) {
	fromcmd (fp+i, md, FROM_ERASE);
	fromunlock (fp+i, md);
	sp[i] = FROM_ERASESECT;
    }

    /* wait for all to finish */
    for (i = 0; i < 4; i++) {
	int stat = frompoll (&sp[i], ~0);
	if (stat != FLASHDEV_OK)
	    status = stat;
    }

    return status;
}

SAFEROUTINE(fromerasesector,
	    (flashcookie_t cp, unsigned int secoffset),
	    (cp, secoffset))


static int
fromreadbytes (flashcookie_t cp, unsigned int offs, 
	       void *mem, unsigned int nb, unsigned int flags)
{
    const struct mandevinfo *md = cp->data;
    void *dp;

    /* check limits */
    if (offs + nb > md->size)
	return FLASHDEV_FAIL;

    if (IS_KVA0 (cp))
	dp = PA_TO_KVA0 (cp->base + offs);
    else
	dp = PA_TO_KVA1 (cp->base + offs);
    memcpy (mem, dp, nb);
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
	

/* read a potentially unaligned from_t from memory */
static __inline__ from_t getmem (const void *mp)
{
    const unsigned char * bp = mp;
#if #endian(big)
    return (bp[0] << 8) | bp[1];
#else
    return (bp[1] << 8) | bp[0];
#endif
}


static int
_reallyprogrambytes (flashcookie_t cp, const struct flashvec *v, int nv,
		    int flags)
{
    const struct mandevinfo *md = cp->data;
    int status = FLASHDEV_OK;
    volatile from_t *fp = PA_TO_KVA1 (cp->base);
    volatile from_t *sp;
    const from_t *mp;
    unsigned int e;
    int i, j;
    int stat;

    while (nv-- != 0) {

	assert ((v->nb & LINEMASK) == 0 && (v->offs & LINEMASK) == 0);

	sp = (from_t *)((char *)fp + v->offs);

	/* erase any sectors that need erasing */
	for (i = 0, e = v->erase; e != 0; i++, e >>= 1) {
	    if (e & 1) {
		DBG ("erase sector [%d] at %06x ...", i, v->soffs);
		fromcmd (fp+i, md, FROM_ERASE);
		fromunlock (fp+i, md);
		sp[i] = FROM_ERASESECT;
	    }
	}

	/* wait for erases to complete */
	for (i = 0, e = v->erase; e != 0; i++, e >>= 1) {
	    if (e & 1) {
		stat = frompoll (sp+i, ~0);
		if (stat != FLASHDEV_OK)
		    status = stat;
	    }
	}	    

	DBG ("program sector from %06x-%06x ...",
	     v->offs, v->offs + v->nb - 1);

	mp = (from_t *)v->mem; 

	/* program up to four devices in parallel */
	for (j = v->nb/sizeof(from_t); j != 0; j -= 4, sp += 4, mp += 4) {
	    unsigned int p = 0;
	    from_t m[4];

	    /* start the programming */
	    for (i = 0; i < 4; i++) {
		m[i] = getmem (&mp[i]);
		if (m[i] != sp[i]) {
		    fromcmd (fp+i, md, FROM_PROGRAM);
		    sp[i] = m[i];
		    p |= 1 << i;
		}
	    }

	    /* wait for the programming to complete */
	    for (i = 0; p != 0; i++, p >>= 1) {
		if (p & 1) {
		    stat = frompoll (&sp[i], m[i]);
		    if (stat != FLASHDEV_OK)
			status = stat;
		}
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
programbytes (flashcookie_t cp, unsigned int offs, 
		    const void *mem, unsigned int nbytes, 
		    int flags)
{
    struct flashvec vec[FROMMAXSECTORS];
    struct flashvec *v = vec;
    unsigned int nb, o;
    int nv = 0;
    int ret;

    /* must be line aligned */
    assert ((offs & LINEMASK) == 0 && (nbytes & LINEMASK) == 0);

    /* construct the flash programming vector */
    for (o = offs, nb = nbytes; nb != 0;) {
	unsigned int soffs, ssize;
	unsigned int n, i, j;
	const from_t *sp, *mp;
	int prot;

	/* find current sector */
	prot = findsector (cp, o, &soffs, &ssize);
	if (prot)
	    return FLASHDEV_PROTECTED;

	/* work out number of bytes that we want to program in this sector */
	n = ssize - (o - soffs);
	if (n > nb)
	    n = nb;

	/* add to vector */
	v->offs = o;
	v->nb = n;
	v->mem = mem;
	v->scratch = 0;
	v->erase = 0;

	/* decide which parts need erasing */
	if (IS_KVA0 (cp))
	    sp = (from_t *) PA_TO_KVA0 (cp->base + offs);
	else
	    sp = (from_t *) PA_TO_KVA1 (cp->base + offs);
	mp = mem;
	for (i = n/sizeof(from_t), j = o/sizeof(from_t); 
	     i != 0 && v->erase != 0xf; i--, j++) {
	    if (getmem (mp++) & ~*sp++)
		v->erase |= 1 << (j & 3);
	}

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
	mips_clean_dcache ((vaddr_t) PA_TO_KVA0 (cp->base) + offs, nbytes);
    
    /* now really do the programming operation */
    ret = reallyprogrambytes (cp, vec, nv, flags);

    for (v = vec; nv != 0; v++, nv--)
	if (v->scratch)
	    free ((char *)v->mem);

    return ret;
}


static flashcookie_t	lineCookie;
static unsigned char	lineBuffer[LINESIZE];
static unsigned int	lineOffset;
static unsigned int	lineFlags;

static int
fromgetline (flashcookie_t cp, unsigned int ho, unsigned int flags)
{
    int stat = FLASHDEV_OK;

    ho &= ~LINEMASK;
    if (lineCookie) {
	if (cp == lineCookie && ho == lineOffset)
	    return stat;
	/* flush old data from buffer to flash */
	stat = programbytes (lineCookie, lineOffset, lineBuffer, LINESIZE, 
			     lineFlags & ~FLASHDEV_PROG_REBOOT);
	lineCookie = (flashcookie_t) 0;
    }
    if (cp && stat == FLASHDEV_OK) {
	/* fill buffer from flash */
	stat = fromreadbytes (cp, ho, lineBuffer, LINESIZE, flags);
	if (stat == FLASHDEV_OK) {
	    lineCookie = cp;
	    lineOffset = ho;
	    lineFlags = flags;
	}
    }
    return stat;
}


/*
 * The user callable programming routine
 */
static int
fromprogrambytes (flashcookie_t cp, unsigned int offs, 
		  const void *mem, unsigned int nb, int flags)
{
    const struct mandevinfo *md = cp->data;
    const unsigned char *m = mem;
    int stat = FLASHDEV_OK;

    /* check limits */
    if (offs + nb > md->size)
	return FLASHDEV_FAIL;

    if (!mem)
	/* flush line buffer */
	return fromgetline (0, 0, 0);

    if (nb == 0)
	return FLASHDEV_OK;

    /* handle leading bytes */
    if (offs & LINEMASK) {
	unsigned int o = offs & LINEMASK;
	unsigned int n = LINESIZE - o;
	if (n > nb)
	    n = nb;
	stat = fromgetline (cp, offs, flags);
	memcpy (&lineBuffer[o], m, n);
	offs += n;
	nb -= n;
	m += n;
    }

    /* handle trailing bytes */
    if ((nb & LINEMASK) && stat == FLASHDEV_OK) {
	unsigned int o = nb & ~LINEMASK;
	unsigned int n = nb & LINEMASK;
	stat = fromgetline (cp, offs + o, flags);
	memcpy (lineBuffer, &m[o], n);
	nb -= n;
    }

    if ((flags & FLASHDEV_PROG_REBOOT) && stat == FLASHDEV_OK)
	/* flush the linebuffer now if we're going to do a reset */
	stat = fromgetline (0, 0, 0);

    /* now do the bulk program (if there's any left) */
    if (nb > 0 && stat == FLASHDEV_OK)
	stat = programbytes (cp, offs, m, nb, flags);

    return stat;
}
