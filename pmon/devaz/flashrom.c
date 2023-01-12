/*
 * share/flashrom.c: Generic Flash ROM support
 * Copyright (c) 1999 Algorithmics Ltd.
 * 
 * Handles most configurations of both uniform and boot sector parts,
 * and supports Intel BCS, AMD and Atmel programming.  
 *
 * Not supported yet:
 *	AMD "unlock bypass" mode
 *	Intel SCS extensions
 *	Sharp extensions
 *
 * To use this module on your board you need to write sbdfrom.c & .h
 * files describing the device geometry.  For example see
 * ../p5064/sbdfrom.* and ../GAL9B/sbdfrom.* 
 */

#ifdef IN_PMON
#include "pmon.h"
#include "sde-compat.h"
#include <alloca.h>
#else
#include <sys/types.h>
#include <mips/cpu.h>
#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include "kit_impl.h"
#endif

#ifndef FROM_DEBUG
#ifndef NDEBUG
#define NDEBUG
#endif
#else
#undef NDEBUG
#endif
#include <assert.h>

#ifdef ITROM
#include <log.h>
#endif

#include "sbd.h"
#ifndef FROM_WIDTH
#include "sbdfrom.h"
#endif
#include "flashdev.h"
#include "flashrom.h"

#define FSECT	(FLASHDEV_PROG_STREAM | _FLASHDEV_SECTOR)
#define FCMD	(FLASHDEV_PROG_STREAM | _FLASHDEV_CMD)

#if FROM_DEBUG > 1
#if defined(ITROM)
#define DBG(fmt, args...) \
	log (LOG_DEBUG, fmt , ## args);
#elif defined(IN_PMON)
#define DBG(fmt, args...) \
	printf (fmt , ## args);
#else
#define DBG(fmt, args...) \
	_mon_printf (fmt , ## args);
#endif
#else
#define DBG(fmt, args...)
#endif

#ifndef FROM_TRIGGER
#define FROM_TRIGGER(msg,addr,want,got)	(void)0
#endif

/*
 * These are the fixed parameters for a particular board.
 */

/* board-specific group size (devices / single-cycle access) */
#ifndef FROM_GROUP
#define FROM_GROUP		FROM_NCOLS
#endif

/* board-specific linesize (bytes / line) */
#ifndef FROM_LINESZ
#define FROM_LINESZ		(FROM_NCOLS * FROM_WIDTH)
#endif

/* linesize mask */
#define FROM_LINEMASK		(FROM_LINESZ - 1)

#ifndef FROM_MAPPABLE
/* device can be read using simple memory address */
#define FROM_MAPPABLE		1
#endif

/*
 * These are the (possibly) variable parameters.
 */

#ifndef FROM_SECSIZE
/* chip-specific (max) sector size */
#define FROM_SECSIZE(fcp)	(fcp)->secsize
#endif

#ifndef FROM_DEVSIZE
/* chip-specific device size */
#define FROM_DEVSIZE(fcp)	(fcp)->devsize
#endif

#ifndef FROM_DEVSIZE_LOG2
/* chip-specific log2(device size) */
#define FROM_DEVSIZE_LOG2(fcp)	(fcp)->devsize_log2
#endif

#ifndef FROM_BANKSIZE
/* board-specific bank size */
#define FROM_BANKSIZE(fcp)	(FROM_DEVSIZE(fcp) * FROM_NCOLS)
#endif

#ifndef FROM_BANKSIZE_LOG2
/* log2(banksize) */
#define FROM_BANKSIZE_LOG2(fcp)	(FROM_DEVSIZE_LOG2(fcp) + FROM_NCOLS_LOG2)
#endif

#ifndef FROM_NBANKS
/* board-specific number of sequential banks */
#define FROM_NBANKS(fcp)	(fcp)->nbanks
#endif

#ifndef FROM_READ_CONTIGUOUS
/* board-specific safe to read flash from contiguously */
#if !defined(fromrline) && !defined(fromrcol)
#define FROM_READ_CONTIGUOUS(flags)	1
#else
#define FROM_READ_CONTIGUOUS(flags)	0
#endif
#endif

#ifndef frompline
/* board-specific line offset munging (program) */
#define frompline(x,flags)	(x)
#endif

#ifndef fromrline
/* board-specific line offset munging (read) */
#define fromrline(x,flags)	(x)
#endif

#ifndef frompcol
/* board-specific column offset munging (program) */
#define frompcol(x,flags)	((x) * FROM_WIDTH)
#endif

#ifndef fromrcol
/* board-specific column offset munging (read) */
#define fromrcol(x,flags)	((x) * FROM_WIDTH)
#endif

#ifndef fromwrenable
/* board-specific write enable/disable */
#define fromwrenable(fcp)	
#define fromwrdisable(fcp)	
#endif

#ifndef fromvppenable
/* board-specific Vpp enable/disable */
#define fromvppenable(fcp)	
#define fromvppdisable(fcp)	
#endif

#ifndef fromprotected
/* board-specific protection test */
#define fromprotected(fcp,offs)	0
#endif

/* unaligned variants of from_t & fromgroup_t */
typedef struct {fromgroup_t v;} __attribute__((packed)) ulfromgroup_t;

#ifndef mips_cycle
#define mips_cycle(count)				\
    do {						\
      register volatile unsigned int __count = (count);	\
      while (__count-- != 0) continue;			\
    } while (0);
#endif

/* 
 * Module entrypoints
 */

static int frominfo (flashcookie_t, unsigned int, struct flashinfo *);
static int fromerasedevice (flashcookie_t);
static int fromerasesector (flashcookie_t, unsigned int);
static int fromprogrambytes (flashcookie_t, unsigned int, const void *, 
			     unsigned int, int);
static int fromreadbytes (flashcookie_t, unsigned int, void *, 
			  unsigned int, unsigned int);


static const struct flashdev fromdev = {
    frominfo,
    fromerasedevice,
    fromerasesector,
    fromprogrambytes,
    fromreadbytes
};



static const struct frommaninfo manuf[] = {
    {"AMD Am",		MAN_AMD},
    {"Fujitsu ",	MAN_FUJITSU},
    {"Atmel AT",	MAN_ATMEL},
    {"Micron MT",	MAN_MICRON},
    {"Intel ",		MAN_INTEL},
    {"SGS Thomson ",	MAN_ST},
    {"Sharp LH",	MAN_SHARP},
    {0},
};
    


/*
 * This is just a list of the devices which Algorithmics has come
 * across, and can be extended as required.  
 *
 * Please send any new entries to sde@algor.co.uk.
 */

static const struct fromdevinfo devinfo[] = {
#if FROM_WIDTH == 1
    /*
     * F004-style devices (8-bit only)
     */

    /* AMD/Fuji */
    {"29F016",	MAN_AMD,	0xad, FBNONE,	0x200000, 0x10000, TYPE_aF004},
    {"29F017",	MAN_AMD,	0x3d, FBNONE,	0x200000, 0x10000, TYPE_aF004},

    /* Intel/Micron */
    {"28F002T",	MAN_INTEL,	0x7c, FBTOP,	0x040000, 0x20000, TYPE_iF004},
    {"28F002B",	MAN_INTEL,	0x7d, FBBOT,	0x040000, 0x20000, TYPE_iF004},
    {"28F004T",	MAN_INTEL,	0x78, FBTOP,	0x080000, 0x20000, TYPE_iF004},
    {"28F004B",	MAN_INTEL,	0x79, FBBOT,	0x080000, 0x20000, TYPE_iF004},
    {"28F008T",	MAN_INTEL,	0x98, FBTOP,	0x100000, 0x20000, TYPE_iF004},
    {"28F008B",	MAN_INTEL,	0x99, FBBOT,	0x100000, 0x20000, TYPE_iF004},

    /* Sharp */
    {"28F008SA",MAN_SHARP,	0xa2, FBNONE,	0x100000, 0x10000, TYPE_iF400},
    {"28F008SC",MAN_SHARP,	0xa6, FBNONE,	0x100000, 0x10000, TYPE_s008C},

    /*
     * F040-style devices  (8-bit only)
     */

    /* AMD/Fuji */
    {"29F040",	MAN_AMD,	0xa4, FBNONE,	0x080000, 0x10000, TYPE_aF040},
    {"29F080",	MAN_AMD,	0xd5, FBNONE,	0x100000, 0x10000, TYPE_aF040},

    /* Atmel */
    {"29C040A",	MAN_ATMEL,	0xa4, FBNONE,	0x080000, 0x00100, TYPE_tC040},
    {"29C040",	MAN_ATMEL,	0x5b, FBNONE,	0x080000, 0x00200, TYPE_tC040},

    /* SGS Thomson */
    {"29F040",	MAN_ST,		0xe2, FBNONE,	0x080000, 0x10000, TYPE_aF040},

#endif

    /* 
     * F400-style devices (8/16 bit)
     */
    /* AMD/Fuji */
    {"29F200T",	MAN_AMD,	0x57, FBTOP,	0x040000, 0x10000, TYPE_aF400},
    {"29F200B",	MAN_AMD,	0x51, FBBOT,	0x040000, 0x10000, TYPE_aF400},
    {"29F400T",	MAN_AMD,	0x23, FBTOP,	0x080000, 0x10000, TYPE_aF400},
    {"29F400B",	MAN_AMD,	0xab, FBBOT,	0x080000, 0x10000, TYPE_aF400},
    {"29F800T",	MAN_AMD,	0xd6, FBTOP,	0x100000, 0x10000, TYPE_aF400},
    {"29F900B",	MAN_AMD,	0x58, FBBOT,	0x100000, 0x10000, TYPE_aF400},

    /* Intel/Micron boot block */
    {"28F200T",	MAN_INTEL,	0x74, FBTOP,	0x040000, 0x20000, TYPE_iF400},
    {"28F200B",	MAN_INTEL,	0x75, FBBOT,	0x040000, 0x20000, TYPE_iF400},
    {"28F400T",	MAN_INTEL,	0x70, FBTOP,	0x080000, 0x20000, TYPE_iF400},
    {"28F400B",	MAN_INTEL,	0x71, FBBOT,	0x080000, 0x20000, TYPE_iF400},
    {"28F800T",	MAN_INTEL,	0x9c, FBTOP,	0x100000, 0x20000, TYPE_iF400},
    {"28F800B",	MAN_INTEL,	0x9d, FBBOT,	0x100000, 0x20000, TYPE_iF400},

    /* Intel FlashFile */
    {"28F016SA",MAN_INTEL,	0xa0, FBNONE,	0x200000, 0x10000, TYPE_iF160},

    /* Intel/Sharp FlashFile */
    {"28F160S",	MAN_SHARP,	0xd0, FBNONE,	0x200000, 0x10000, TYPE_iF160},
    {"28F320S",	MAN_SHARP,	0xd4, FBNONE,	0x400000, 0x10000, TYPE_iF160},

    /* Intel StrataFlash */
    {"28F320J",	MAN_INTEL,	0x14, FBNONE,	0x400000, 0x20000, TYPE_iF160},
    {"28F640J",	MAN_INTEL,	0x15, FBNONE,	0x800000, 0x20000, TYPE_iF160},

    /* Micron (non-Intel compatible) */
    /* the first two appear to be obsolete */
    {"28LF400T", MAN_MICRON,	0x30, FBTOP,	0x080000, 0x20000, TYPE_iF400},
    {"28LF400B", MAN_MICRON,	0x31, FBBOT,	0x080000, 0x20000, TYPE_iF400},
    {"28SF400T", MAN_MICRON,	0xb0, FBTOP,	0x080000, 0x20000, TYPE_iF400},
    {"28SF400B", MAN_MICRON,	0xb1, FBBOT,	0x080000, 0x20000, TYPE_iF400},

    /* Sharp */
    {"28F016SU", MAN_SHARP,	0x88, FBNONE,	0x200000, 0x10000, TYPE_s016U},
    /* Sharp 28F032SU is literally two banks of 28F016SU */

    /* 
     * LV400-style devices  (8/16 bit)
     */

    /* AMD/Fuji */
    {"29LV400B", MAN_AMD,	0xba, FBBOT,	0x080000, 0x10000, TYPE_LV400},
    {"29LV800B", MAN_AMD, 	0x5b, FBBOT,	0x100000, 0x10000, TYPE_LV400},
    {"29LV800T", MAN_AMD,	0xda, FBTOP,	0x100000, 0x10000, TYPE_LV400},
    {"29LV160B", MAN_AMD,	0x49, FBBOT,	0x200000, 0x10000, TYPE_LV400},
    {"29LV160T", MAN_AMD,	0xc4, FBTOP,	0x200000, 0x10000, TYPE_LV400},

    /* AMD 29DL devices (8/16 bit, asymettric banks) */
    {"29DL322T",MAN_AMD,	0x55, FBTOP,	0x400000, 0x10000, TYPE_aDL,
     0x80000},
    {"29DL322B",MAN_AMD,	0x56, FBBOT,	0x400000, 0x10000, TYPE_aDL,
     0x80000},
    {"29DL323T",MAN_AMD,	0x50, FBTOP,	0x400000, 0x10000, TYPE_aDL,
     0x100000},
    {"29DL323B",MAN_AMD,	0x53, FBBOT,	0x400000, 0x10000, TYPE_aDL,
     0x100000},

    {0}
};



#if defined(ITROM)
/* no heap - allocate memory from a static buffer */
#define malloc	itmalloc

#define ITMAXMEMBUF 512
static long itmembuf[ITMAXMEMBUF];
static int itmembufnext;

static void *
itmalloc (size_t size)
{
    long *p;

    size = (size + sizeof(long) - 1) / sizeof(long);
    if (itmembufnext + size > ITMAXMEMBUF) {
	log (LOG_CRIT, "FLASH malloc: want %d words, only got %d\n", 
	     size, ITMAXMEMBUF - itmembufnext);
	return 0;
    }
    p = &itmembuf[itmembufnext];
    itmembufnext += size;
    DBG ("FLASH malloc: allocated %d words, %d remaining\n", 
	 size, ITMAXMEMBUF - itmembufnext);
    return (void *)p;
}
#endif /* ITROM */



#if FROM_DEBUG > 1 || defined(FROM_NEVER_SELF_PROGRAM)

/* simple static routines */
#define INLINE

#define SAFEROUTINE(name,protoargs,callargs) \
static int name protoargs

#define ENDROUTINE(name)

#else

/* worker routines must be inline */
#define INLINE __inline__

/* in case we ever run code from flash... copy programming code to RAM */
#define SAFEROUTINE(name,protoargs,callargs) \
static int _safe_##name protoargs; \
static void _end_##name (void); \
static int name protoargs \
{ \
    static int (*fn) protoargs; \
    if (!fn) {\
        size_t codesize = (char *)_end_##name - (char *)_safe_##name; \
        fn = malloc (codesize); \
        if (!fn) return FLASHDEV_NOMEM; \
        memcpy (fn, _safe_##name, codesize); \
        mips_clean_cache ((vaddr_t)fn, codesize); \
    } \
    return (*fn) callargs; \
} \
static int _safe_##name protoargs

#define ENDROUTINE(name) \
static void _end_##name (void) {}

#endif



/* get address of line in flash rom */
#define fromlineaddr(fcp, bank, addr, flags) \
    (fcp->pbase + frompline (fromoffs(fcp, bank, addr, flags), flags))

/* get address of word in flash rom */
#define fromaddr(fcp, bank, addr, col, flags) \
    (fromlineaddr(fcp, bank, addr, flags) + frompcol(col, flags))

/* do any flag dependent munging on the data */
#ifndef fromconvert
#define fromconvert(toflgs, fromflgs, val) (val)
#endif

/* write word group to flash address */
#ifndef frompoke
#define frompoke(addr, pval, flags) \
     *(volatile fromgroup_t *)(addr) = (pval)
#endif

/* read word group from flash address */
#ifndef frompeek
#define frompeek(addr, flags) \
    *(volatile fromgroup_t *)(addr)
#endif

/* write word to flash index */
#define fromput(fcp, bank, col, addr, val, flags) \
    frompoke (fromaddr(fcp, bank, addr, col, flags), val, flags)

/* read word from flash index */
#define fromget(fcp, bank, col, addr, flags) \
    frompeek (fromaddr(fcp, bank, addr, col, flags), flags)


/* convert byte offset into <bank, col, addr> */
static INLINE unsigned int
foffs2addr (fromcookie_t fcp, unsigned int offs,
	    unsigned int *bankp, unsigned int *colp)
{
    unsigned int banksize_log2 = FROM_BANKSIZE_LOG2(fcp);
    unsigned int addr;
    unsigned int bank;

    bank = offs >> banksize_log2;
    offs &= (1 << banksize_log2) - 1;

#if 0
    if (fcp->flags & FROMFLG_AM29DL) {
	bank *= 2;
	if (offs > fcp->dlbanksz[0]) {
	    offs -= fcp->dlbanksz[0];
	    ++bank;
	}
    }
#endif

    addr = offs / FROM_WIDTH;

    if (bankp)
	*bankp = bank;

    if (colp) {
	unsigned int col = addr & (FROM_NCOLS - 1);
	assert ((col & (FROM_GROUP - 1)) == 0);
	*colp = col;
    }

    return (addr / FROM_NCOLS);
}

/* convert <bank, addr> indices to byte offset */
static INLINE unsigned int
fromoffs (fromcookie_t fcp, unsigned int bank, unsigned int addr, 
	  unsigned int flags) 				\
{
    unsigned int bankoffs;

#if 0
    if (fcp->flags & FROMFLG_AM29DL) {
	bankoffs = (bank / 2) << FROM_BANKSIZE_LOG2(fcp);
	if (bank & 1)
	    bankoffs += fcp->dlbanksz[0];
    }
    else
#endif
    {
	bankoffs = bank << FROM_BANKSIZE_LOG2(fcp);
    }
    return bankoffs + (addr * FROM_LINESZ);
}



static INLINE void
fromcmd_amd (fromcookie_t fcp, unsigned int bank, unsigned int col, 
	     unsigned int flags, fromdev_t cmd, int unlock_again)
{
    from_pt cmd1 = fromaddr (fcp, bank, fcp->cmd1_offs, col, 
			     flags | _FLASHDEV_CMD);
#if FROM_FAMILY_AMD
    from_pt cmd2 = fromaddr (fcp, bank, fcp->cmd2_offs, col, 
			     flags | _FLASHDEV_CMD);
    frompoke (cmd1, fromgroup (AMD_CMD1_DATA), FCMD);
    frompoke (cmd2, fromgroup (AMD_CMD2_DATA), FCMD);
#endif
    frompoke (cmd1, fromgroup (cmd), FCMD);
    if (fcp->flags & FROMFLG_DELAY) {
	if (IS_KVA0(&cmd1)) {
	    mips_cycle (CACHEMS (20));
	}
	else {
	    mips_cycle (RAMMS (20));
	}
    }
#if FROM_FAMILY_AMD
    if (unlock_again) {
	frompoke (cmd1, fromgroup (AMD_CMD1_DATA), FCMD);
	frompoke (cmd2, fromgroup (AMD_CMD2_DATA), FCMD);
    }
#endif
}


static INLINE void
fromreset (fromcookie_t fcp, unsigned int bank, unsigned int col,
	   unsigned int flags)
{
    from_pt fp = fromaddr (fcp, bank, 0, col, flags);
    volatile fromgroup_t dummy;

    /* We may be polling to find the Flash type, and we don't
       know yet what family it is, so do every sort of reset
       for all supported families. NB poll code sets ATMEL
       flag by default. */
#if FROM_FAMILY_AMD
    if (fcp->flags & FROMFLG_ATMEL)
	fromcmd_amd (fcp, bank, col, flags, FROMCMD_AMD_RESET, 0);
    else
	/* XXX not safe on Sharp flash (0xf0 is the "sleep" command)! */
	frompoke (fp, fromgroup (FROMCMD_AMD_RESET), FCMD);
#endif
#if FROM_FAMILY_INTEL
    frompoke (fp, fromgroup (FROMCMD_IBCS_RESET), FCMD);
    frompoke (fp, fromgroup (FROMCMD_IBCS_CLEARSR), FCMD);
#endif
    /* paranoid write buffer flush, to make sure we don't return to
       code running on a device which is still in program/erase mode. */
    dummy = frompeek (fp, FCMD);
}



static INLINE int
frompoll_ibcs (fromcookie_t fcp, from_pt fp, fromgroup_t expect, 
	       unsigned int flags)
{
    int status = FLASHDEV_OK;
#if FROM_FAMILY_INTEL
    fromgroup_t poll;

    while (1) {
	poll = frompeek (fp, FCMD);
	if ((poll & fromgroup(ICSR_READY)) == fromgroup(ICSR_READY))
	    break;
    }
    frompoke (fp, fromgroup (FROMCMD_IBCS_RESET), FCMD); 

    if (poll & fromgroup (ICSR_WRITE_ERR|ICSR_ERASE_ERR|ICSR_VPP_ERR)) {
	frompoke (fp, fromgroup (FROMCMD_IBCS_CLEARSR), FCMD);
	FROM_TRIGGER ("ERR", fp, expect, poll);
	DBG ("PROG ERR @%x (want %x poll %x got %x)\n",
	     fp, expect, poll, frompeek (fp, flags));
	status = FLASHDEV_FATAL;
    }
    else if ((poll = frompeek (fp, flags)) != expect) {
	FROM_TRIGGER ("ERR", fp, expect, poll);
	DBG ("POLL ERROR @%x (want %x got %x)\n",
	     fp, expect, poll);
	status = FLASHDEV_FATAL;
    }
#endif
    return status;
}


/* AMD polling */
static INLINE int
frompoll_amd (fromcookie_t fcp, from_pt fp, fromgroup_t expect,
	      unsigned int flags)
{
    int status = FLASHDEV_OK;
#if FROM_FAMILY_AMD
    unsigned int busy = (1 << FROM_GROUP) - 1;
    unsigned int timeout = 0;
    fromgroup_t poll;

    expect = fromconvert (FCMD, flags, expect);
    while (1) {
	unsigned int which, word;

	poll = frompeek (fp, FCMD);
	which = 1;
	foreach_word (word) {
	    if (busy & which) {
#if #endian(big) 
		unsigned int bitpos = (FROM_GROUP - word - 1) * FROM_WIDTH * 8;
#else
		unsigned int bitpos = word * FROM_WIDTH * 8;
#endif
		if (((poll ^ expect) & (AMD_DQPOLL << bitpos)) == 0) {
		    /* finished */
		    busy &= ~which;
		}
		else if (!(fcp->flags & FROMFLG_ATMEL)) {
		    if (timeout & which) {
			/* last chance failed */
			FROM_TRIGGER ("TIMEOUT", fp, expect, poll);
			DBG ("TIMEOUT @%x (want %x poll %x)\n",
			     fp, expect, poll);
			status = FLASHDEV_FATAL;
			busy &= ~which;
		    }
		    else if (poll & (AMD_DQTIMEEXCEEDED << bitpos)) {
			/* timed out, one more poll allowed */
			timeout |= which;
		    }
		}
	    }

	    which <<= 1;
	}
	if (busy == 0)
	    break;
    }

    if (status != FLASHDEV_OK)
	frompoke (fp, fromgroup (FROMCMD_AMD_RESET), FCMD); 
    else if ((poll = frompeek (fp, FCMD)) != expect) {
	FROM_TRIGGER ("CHECK", fp, expect, poll);
	DBG ("POLL ERROR @%x (want %x got %x)\n",
	     fp, expect, poll);
	status = FLASHDEV_FATAL;
    }
#endif
    return status;
}



/* read manufacture and device id from chip */
SAFEROUTINE(fromautoselect, (fromcookie_t fcp, unsigned int offs, 
			     unsigned char *manp, unsigned char *devp),
	    (fcp, offs, manp, devp))
{
#if FROM_WIDTH == 1
    int iashift = (fcp->flags & FROMFLG_BIWIDE) ? 1 : 0;
#else
    int iashift = 0;
#endif
    unsigned int bank, col, addr;
    from_pt mp, dp;
    fromgroup_t man, dev;
    fromgroup_t prot;

    fromwrenable (fcp);

    addr = foffs2addr (fcp, offs, &bank, &col);

    /* start with a reset to get in sync */
    fromreset (fcp, bank, col, FCMD);

    /* always do AMD command sequence, this will work for IBCS too */
    fromcmd_amd (fcp, bank, col, FCMD, FROMCMD_AUTOSELECT, 0);

    /* get manufacturer and device id */
    mp = fromaddr (fcp, bank, 0 << iashift, col, FCMD);
    dp = fromaddr (fcp, bank, 1 << iashift, col, FCMD);
    man = frompeek (mp, FCMD);
    dev = frompeek (dp, FCMD);

    if (fromprotected (fcp, offs))
	/* protected externally */
	prot = 1;
    else if (fcp->flags & FROMFLG_PROT) {
	/* read sector protect status (standard) */
	addr &= ~(((fcp->secsize / FROM_WIDTH) - 1) >> iashift);
	prot = fromget (fcp, bank, col, addr | (2 << iashift), FCMD);
	prot = (prot & fromgroup(0x1)) != 0;
    }
    else if (fcp->flags & FROMFLG_SPROT) {
	/* read sector protect status (sharp) */
	from_pt gsr;
	frompoke (mp, fromgroup (FROMCMD_IBCS_RESET), FCMD);
	frompoke (mp, fromgroup (FROMCMD_SHARP_UPSTAT), FCMD);
	frompoke (mp, fromgroup (FROMCMD_IBCS_CONFIRM), FCMD);
	frompoke (mp, fromgroup (FROMCMD_SHARP_RXSR), FCMD);
	gsr = fromaddr (fcp, bank, 2 << iashift, col, FCMD);
	while ((frompeek (gsr, FCMD) & fromgroup (SGSR_READY))
	       != fromgroup (SGSR_READY))
	    continue;
	addr &= ~((fcp->secsize - 1) >> iashift);
	prot = fromget (fcp, bank, col, addr | (1 << iashift), FCMD);
	prot = (prot & fromgroup (SBSR_UNLOCK)) != fromgroup (SBSR_UNLOCK);
    }
    else {
	/* no protect status available */
	prot = 0;
    }

    fromreset (fcp, bank, col, FCMD);

    fromwrdisable (fcp);

    if (man != fromgroup (man) || dev != fromgroup (dev)
	|| (man == frompeek (mp, FCMD) && dev == frompeek (dp, FCMD)))
	/* parallel devices must match - and should now
	   be different to before */
	man = dev = 0;

    if (manp)
	*manp = man;

    if (devp)
	*devp = dev;

    return prot;
}
ENDROUTINE(fromautoselect)



/*
 * Return the starting offset and total size of the sector
 * containing the given offset
 */

static int
findsector (fromcookie_t fcp, unsigned int offs, 
	    unsigned int *soffsp, unsigned int *ssizep)
{
    static const unsigned int bootsecs_std[] = {0x4000, 0x2000, 0x2000, 0};
    static const unsigned int bootsecs_dl[] = {0x2000, 0x2000, 0x2000, 0x2000,
    					       0x2000, 0x2000, 0x2000, 0};
    const struct fromdevinfo *dv = fcp->dv;
    const unsigned int *bootsecs;
    unsigned int banksize = FROM_BANKSIZE(fcp);
    unsigned int secsize = FROM_SECSIZE(fcp) * FROM_NCOLS;
    unsigned int bootspace = secsize;
    unsigned int nboot, nsec;
    unsigned int bank, addr;
    unsigned int soffs;
    int secno, ssize;

    /* convert to device address */
    addr = foffs2addr (fcp, offs, &bank, NULL);

    if (bank >= FROM_NBANKS(fcp)) {
	/* out of range */
	if (soffsp) *soffsp = 0;
	if (ssizep) *ssizep = 0;
	return -1;
    }

    /* convert to byte offset in bank, not word offset in column */
    offs = addr * FROM_LINESZ;

#if 0
    if (fcp->flags & FROMFLG_AM29DL) {
	/* Am29DL has complicated internal banks, but simple boot sectors */
	unsigned int secmask = secsize - 1;
	unsigned int devno = bank >> 1;

	/* find main bank base sector number */
	soffs = FROM_DEVSIZE(fcp) * devno;
	secno = (FROM_DEVSIZE(fcp) / FROM_SECSIZE(fcp) + 7) * devno;

	/* adjust for sub-bank base */
	if (bank == 1) {
	    soffs += fcp->dlbanksz[0];
	    secno += fcp->dlbanksz[0] / secsize;
	    if (dv->boot == FBBOT)
		secno += 7;
	}

	/* calculate major sector base */
	secno += offs / secsize;
	soffs += offs & ~secmask;

	/* adjust for boot sectors */
	if ((dv->boot == FBTOP && bank != 0 
	     && offs > fcp->dlbanksz[1] - secsize)
	    || (dv->boot == FBBOT && bank == 0 
		&& offs < secsize)) {
	    unsigned int bsec = (offs & secmask) / (8192 * FROM_NCOLS);
	    secno += bsec;
	    soffs += bsec * 8192 * FROM_NCOLS;
	}
    }
    else
#endif
    {
	if (fcp->flags & FROMFLG_AM29DL) {
	    bootsecs = bootsecs_dl;
	    nboot = 8 - 1;
	}
	else {
	    bootsecs = bootsecs_std;
	    nboot = 4 - 1;
	}

	/* number of sectors per bank */
	nsec = FROM_DEVSIZE(fcp) / FROM_SECSIZE(fcp);
	if (dv->boot != FBNONE)
	    nsec += nboot;

	if (dv->boot == FBTOP && offs >= banksize - bootspace) {
	    /* in top boot sector */
	    soffs = banksize;
	    secno = nsec;
	    while (ssize = (*bootsecs++ * FROM_NCOLS)) {
		/* decrement sector base by sector size */
		soffs -= ssize;
		secno--;
		if (offs >= soffs)
		    break;
	    }
	    if (!ssize) {
		/* lowest boot sector */
		unsigned int noffs = (banksize - bootspace);
		ssize = soffs - noffs;
		soffs = noffs;
		secno--;
	    }
	}
	else if (dv->boot == FBBOT && offs < bootspace) {
	    /* in bottom boot sector */
	    soffs = 0;
	    secno = 0;
	    while (ssize = (*bootsecs++ * FROM_NCOLS)) {
		if (offs < soffs + ssize)
		    break;
		soffs += ssize;
		secno++;
	    }
	    if (!ssize) {
		/* last boot sector */
		ssize = bootspace - soffs;
	    }
	}
	else {
	    /* in normal sector */
	    soffs = offs & ~(secsize - 1);
	    ssize = secsize;
	    secno = soffs / secsize;
	    if (dv->boot == FBBOT)
		secno += nboot;
	}
	
	/* adjust offset and sector number */
	soffs += (bank * banksize);
	secno += (bank * nsec);
    }

    if (soffsp) *soffsp = soffs;
    if (ssizep) *ssizep = ssize;
    return (secno);
}


static int
sectorinfo (fromcookie_t fcp, unsigned int offs, 
	    unsigned int *soffsp, unsigned int *ssizep)
{
    int secno, nsec;

    /* convert to device address */
    secno = findsector (fcp, offs, soffsp, ssizep);
    if (secno < 0)
	return 1;

    nsec = FROM_DEVSIZE (fcp) / FROM_SECSIZE (fcp);
    if (fcp->dv->boot != FBNONE)
	nsec += (fcp->flags & FROMFLG_AM29DL) ? 7 : 3;
    nsec *= FROM_NBANKS (fcp);

    /* create cache of sector protect status */
    if (!fcp->protect) {
	unsigned int i, soffs, ssize;

	((struct fromcookie *)fcp)->protect = malloc (nsec);
	if (!fcp->protect)
	    return 1;
	
	for (soffs = i = 0; soffs < fcp->size; soffs += ssize, i++) {
	    unsigned int so, col, prot = 0;
	    (void) findsector (fcp, soffs, NULL, &ssize);
	    foreach_sec (fcp, so) {
		foreach_group (col) {
		    prot |= fromautoselect (fcp, 
					    soffs + so + frompcol (col, FSECT),
					    NULL, NULL);
		}
	    }
	    assert (i < nsec);
	    fcp->protect[i] = prot;
	}
	assert (i == nsec);
    }

    assert (secno < nsec);
    return fcp->protect[secno];
}


static int
frominfo (flashcookie_t cp, unsigned int offs, struct flashinfo *info)
{
    fromcookie_t fcp = (fromcookie_t) cp;

    /* construct name from manufacturer and device name */
    strcpy (info->name, fcp->mf->name ? fcp->mf->name : "??? ");
    strcat (info->name, fcp->dv->name);

    /* board specific parameters */
    info->base = KVA_TO_PA (fcp->pbase);
    info->mapbase = fcp->mapbase;
    info->unit = FROM_LINESZ;

    /* flash device specific parameters */
    info->size =  fcp->size;
    info->maxssize = FROM_SECSIZE(fcp) * FROM_NCOLS;

    /* get specified sector information */
    info->sprot = sectorinfo (fcp, offs, &info->soffs, &info->ssize);

    return FLASHDEV_OK;
}



SAFEROUTINE(fromerasedevice, (flashcookie_t cp),
	    (cp))
{
    fromcookie_t fcp = (fromcookie_t) cp;
    unsigned int bank, col;
    int status = FLASHDEV_FAIL;

    if (fcp->flags & (FROMFLG_AMD | FROMFLG_ATMEL)) {
	fromvppenable (fcp);
	fromwrenable (fcp);
	
	/* erase all in parallel */
	for (bank = 0; bank < FROM_NBANKS(fcp); bank++) {
	    foreach_group (col) {
		fromcmd_amd (fcp, bank, col, FSECT, FROMCMD_AMD_ERASE, 0);
		fromcmd_amd (fcp, bank, col, FSECT, FROMCMD_AMD_ERASECHIP, 0);
	    }
	}
	
	/* wait for all to finish */
	status = FLASHDEV_OK;
	for (bank = 0; bank < FROM_NBANKS(fcp); bank++) {
	    from_pt fp = fromlineaddr (fcp, bank, 0, FSECT);
	    foreach_group (col) {
		int stat;
		stat = frompoll_amd (fcp, fp + frompcol (col, FSECT),
				     ~0, FSECT);
		if (stat != FLASHDEV_OK)
		    status = stat;
	    }
	}
	
	fromwrdisable (fcp);
	fromvppdisable (fcp);
    }
    return status;
}
ENDROUTINE(fromerasedevice)



SAFEROUTINE(fromerasesec, 
	    (flashcookie_t cp, unsigned int secoffset),
	    (cp, secoffset))
{
    fromcookie_t fcp = (fromcookie_t) cp;
    unsigned int bank, col, addr, soffs;
    int status = FLASHDEV_OK;
    from_pt fp;

    addr = foffs2addr (fcp, secoffset, &bank, NULL);
    if (bank > FROM_NBANKS(fcp))
	return FLASHDEV_FAIL;

    fromvppenable (fcp);
    fromwrenable (fcp);

    foreach_sec (fcp, soffs) {
	fp = fromlineaddr (fcp, bank, addr, FSECT) 
	    + frompline (soffs, FSECT);

	if (fcp->flags & FROMFLG_IBCS) {
	    /* erase each column */
	    foreach_group (col) {
		from_pt cp = fp + frompcol (col, FSECT);
		frompoke (cp, fromgroup (FROMCMD_IBCS_ERASE), FCMD);
		frompoke (cp, fromgroup (FROMCMD_IBCS_CONFIRM), FCMD);
	    }
	}
	else if (fcp->flags & FROMFLG_ATMEL) {
	    /* reprogram each column with ~0 (implied erase) */
	    foreach_group (col) {
		from_pt cp = fp + frompcol (col, FSECT);
		fromcmd_amd (fcp, bank, col, FSECT, FROMCMD_AMD_PROGRAM, 0);
		frompoke (cp, ~0, FCMD);
	    }
	}
	else if (fcp->flags & FROMFLG_AMD) {
	    /* erase each column */
	    foreach_group (col) {
		from_pt cp = fp + frompcol (col, FSECT);
		fromcmd_amd (fcp, bank, col, FSECT, FROMCMD_AMD_ERASE, 1);
		frompoke (cp, fromgroup (FROMCMD_AMD_ERASESECT), FCMD);
	    }
	}
	
	/* wait for all columns to finish */
	foreach_group (col) {
	    from_pt cp = fp + frompcol (col, FSECT);
	    int stat;
	    if (fcp->flags & FROMFLG_IBCS)
		stat = frompoll_ibcs (fcp, cp, ~0, FSECT);
	    else if (fcp->flags & (FROMFLG_AMD | FROMFLG_ATMEL))
		stat = frompoll_amd (fcp, cp, ~0, FSECT);
	    else
		stat = FLASHDEV_FAIL;
	    if (stat != FLASHDEV_OK)
		status = stat;
	}
    }

    fromwrdisable (fcp);
    fromvppdisable (fcp);
    return status;
}
ENDROUTINE(fromerasesec)


static int 
fromerasesector (flashcookie_t cp, unsigned int offset)
{
    fromcookie_t fcp = (fromcookie_t) cp;
    unsigned int secoffset;

    if (sectorinfo (fcp, offset, &secoffset, NULL))
	return FLASHDEV_PROTECTED;
    return fromerasesec (cp, secoffset);
}



static int
fromreadbytes (const flashcookie_t cp, unsigned int offs, 
	       void *buf, unsigned int nb, unsigned int flags)
{
    fromcookie_t fcp = (fromcookie_t) cp;
    unsigned char *mem = buf;
    unsigned char align[FROM_LINESZ];

    /* check limits */
    if (offs + nb > fcp->size)
	return FLASHDEV_FAIL;

    if (FROM_MAPPABLE && FROM_READ_CONTIGUOUS (flags)) {
	/* no munging - copy bytes quickly */
	memcpy (mem, (void *)(fcp->rbase + offs), nb);
	return FLASHDEV_OK;
    }

    while (nb != 0) {
	unsigned int n;

	if (offs & FROM_LINEMASK) {
	    unsigned int o = offs & FROM_LINEMASK;
	    fromreadbytes (cp, offs - o, align, FROM_LINESZ, flags);
	    n = FROM_LINESZ - o;
	    if (n > nb)
		n = nb;
	    memcpy (mem, &align[o], n);
	}
	else if (nb < FROM_LINESZ) {
	    fromreadbytes (cp, offs, align, FROM_LINESZ, flags);
	    memcpy (mem, align, n = nb);
	}
	else {
	    /* undo the effects of any word-packing or address munging */
	    ulfromgroup_t *mp = (ulfromgroup_t *)mem;
	    from_pt rbase = fcp->rbase;
	    unsigned int o, col, nl;
	    for (nl = nb / FROM_LINESZ, o = offs; nl != 0;
		 nl--, o += FROM_LINESZ) {
		from_pt lp = rbase + fromrline (o, flags);
		foreach_group (col) {
		    from_pt cp = lp + fromrcol (col, flags);
		    (mp++)->v = frompeek (cp, flags);
		}
	    }
	    n = nb & ~FROM_LINEMASK;
	}
	offs += n;
	mem += n;
	nb -= n;
    }
    return FLASHDEV_OK;
}



/* We build a vector of these, each representing the data to be
   programmed into one sector. */
struct fromvec {
    unsigned int 	offs; 
    const void *	mem;
    unsigned int 	nb;
    unsigned int	erase;
    int			scratch;
};
	


SAFEROUTINE(fromprogramvec, (fromcookie_t fcp, 
			     const struct fromvec *v, 
			     int nv, int flags),
	    (fcp, v, nv, flags))
{
    int status = FLASHDEV_OK;
    unsigned int bank, col, addr;
    unsigned int e, n;
    from_pt fp;
    int stat;

    fromvppenable (fcp);
    fromwrenable (fcp);

    for (; nv != 0; v++, nv--) {
	const ulfromgroup_t *mp = (ulfromgroup_t *)v->mem; 

	assert ((v->nb & FROM_LINEMASK) == 0 
		&& (v->offs & FROM_LINEMASK) == 0);

	addr = foffs2addr (fcp, v->offs, &bank, NULL);

	DBG ("program %06x-%06x (bank %d addr 0x%x emsk 0x%x) ...",
	     v->offs, v->offs + v->nb - 1, bank, addr, v->erase);

	if (fcp->flags & FROMFLG_ATMEL) {

	    /* start the programming */
	    foreach_group (col) {
		fromcmd_amd (fcp, bank, col, flags, FROMCMD_AMD_PROGRAM, 0);
	    }

	    /* program whole sectors */
	    for (n = v->nb / FROM_LINESZ; n != 0; n--, addr++) {
		fp = fromlineaddr (fcp, bank, addr, flags);
		foreach_group (col) {
		    from_pt cp = fp + frompcol (col, flags);
		    frompoke (cp, (mp++)->v, flags);
		}
	    }

	    /* back up one line and poll for completion of last words */
	    mp -= FROM_NCOLS / FROM_GROUP; 
	    fp = fromlineaddr (fcp, bank, addr - 1, flags);
	    foreach_group (col) {
		from_pt cp = fp + frompcol (col, flags);
		stat = frompoll_amd (fcp, cp, (mp++)->v, flags);
		if (stat != FLASHDEV_OK)
		    status = stat;
	    }
	}
	else {
	    /* erase any sectors that need erasing */
	    unsigned int soffs;

	    foreach_sec (fcp, soffs) {
		fp = fromlineaddr (fcp, bank, addr, FSECT)
		    + frompline (soffs, FSECT);

		e = v->erase;
		foreach_group (col) {
		    if (e & 1) {
			from_pt cp = fp + frompcol (col, FSECT);
			DBG ("erase sector [col %d-%d] at %06x ...\n", 
			     col, col + FROM_GROUP - 1, v->offs + soffs);
			if (fcp->flags & FROMFLG_IBCS) {
			    frompoke (cp, fromgroup (FROMCMD_IBCS_ERASE), FCMD);
			    frompoke (cp, fromgroup (FROMCMD_IBCS_CONFIRM), FCMD);
			}
			else if (fcp->flags & FROMFLG_AMD) {
			    fromcmd_amd (fcp, bank, col, FSECT,
					 FROMCMD_AMD_ERASE, 1);
			    frompoke (cp, fromgroup (FROMCMD_AMD_ERASESECT), FCMD);
			}
		    }
		    e >>= FROM_GROUP;
		}
		
		/* wait for erases to complete */
		e = v->erase;
		foreach_group (col) {
		    if (e & 1) {
			from_pt cp = fp + frompcol (col, FSECT);
			if (fcp->flags & FROMFLG_IBCS)
			    stat = frompoll_ibcs (fcp, cp, ~0, FSECT);
			else if (fcp->flags & FROMFLG_AMD)
			    stat = frompoll_amd (fcp, cp, ~0, FSECT);
			else
			    stat = FLASHDEV_FAIL;
			if (stat != FLASHDEV_OK)
			    status = stat;
		    }
		    e >>= FROM_GROUP;
		}	    
	    }

	    /* program one line at a time in parallel */
	    for (n = v->nb / FROM_LINESZ; n != 0; n--, addr++) {
		unsigned int prog = 0;
		
		fp = fromlineaddr (fcp, bank, addr, flags);

		/* start the programming */
		foreach_group (col) {
		    from_pt cp = fp + frompcol (col, flags);
		    fromgroup_t nv = (mp++)->v;
		    if (frompeek (cp, flags) != nv) {
			if (fcp->flags & FROMFLG_IBCS)
			    frompoke (cp, fromgroup (FROMCMD_IBCS_PROGRAM), FCMD);
			else if (fcp->flags & FROMFLG_AMD)
			    fromcmd_amd (fcp, bank, col, flags, 
					 FROMCMD_AMD_PROGRAM, 0);
			frompoke (cp, nv, flags);
			prog |= 1 << col;
		    }
		}

		if (prog == 0)
		    continue;

		/* wait for the programming to complete */
		mp -= FROM_NCOLS / FROM_GROUP;
		foreach_group (col) {
		    if (prog & 1) {
			from_pt cp = fp + frompcol (col, flags);
			fromgroup_t nv = mp->v;
			if (fcp->flags & FROMFLG_IBCS)
			    stat = frompoll_ibcs (fcp, cp, nv, flags);
			else if (fcp->flags & FROMFLG_AMD)
			    stat = frompoll_amd (fcp, cp, nv, flags);
			else
			    stat = FLASHDEV_FAIL;
			if (stat != FLASHDEV_OK)
			    status = stat;
		    }
		    ++mp;
		    prog >>= FROM_GROUP;
		}
	    }
	}

	DBG ("done (%d)\n", status);
    }

    fromwrdisable (fcp);
    fromvppdisable (fcp);

    if (flags & FLASHDEV_PROG_REBOOT)
	/* if we've overwritten the prom code, we must reboot */
	((volatile void (*)(void))PA_TO_KVA1(BOOTPROM_BASE)) ();

    return status;
}
ENDROUTINE(fromprogramvec)



static unsigned int
fromerasemask (fromcookie_t fcp, const struct fromvec *v, int flags)
{
    const ulfromgroup_t *mp = (ulfromgroup_t *)v->mem;
    unsigned int erase = 0;
    unsigned int bank, col, addr, lines;

    addr = foffs2addr (fcp, v->offs, &bank, NULL);
    for (lines = v->nb / FROM_LINESZ; lines != 0; lines--, addr++) {
	from_pt fp = fromlineaddr (fcp, bank, addr, flags);
	foreach_group (col) {
	    fromgroup_t ov;
	    from_pt cp;
	    cp = fp + frompcol (col, flags);
	    ov = frompeek (cp, flags);
	    if ((mp++)->v & ~ov) {
		erase |= ((1 << FROM_GROUP) - 1) << col;
		if (erase == (1 << FROM_NCOLS) - 1)
		    return erase;	/* all erase column bits set */
	    }
	}
    }
    return erase;
}


static void
fromcleancache (fromcookie_t fcp, const struct fromvec *v, int flags)
{
    from_pt rbase;

    if (IS_KVA0 (fcp->rbase))
	rbase = fcp->rbase;
    else if (FROM_MAPPABLE && fcp->mapbase != ~0)
	rbase = PA_TO_KVA0 (fcp->mapbase);
    else
	return;
	
    if (FROM_READ_CONTIGUOUS (flags)) {
	mips_clean_cache ((vaddr_t)(rbase + v->offs), v->nb);
    }
    else {
	unsigned int nl, o;
	for (nl = v->nb / FROM_LINESZ, o = v->offs; nl != 0;
	     nl--, o += FROM_LINESZ) {
	    from_pt lp = rbase + fromrline (o, flags);
	    mips_clean_cache ((vaddr_t) lp, FROM_LINESZ);
	}
    }
}


static int
fromprogramaligned (fromcookie_t fcp, unsigned int offs, 
		    const void *buf, unsigned int nbytes, 
		    int flags)
{
    const char *mem = buf;
    struct fromvec *vec, *v;
    unsigned int ssize, nb, o;
    int nvec, nv = 0;
    int ret = FLASHDEV_OK;

    if (nbytes == 0)
	return ret;

    /* must be line aligned */
    assert ((offs & FROM_LINEMASK) == 0 
	    && (nbytes & FROM_LINEMASK) == 0);

    /* calc number of sectors spanned */
    ssize = FROM_SECSIZE(fcp) * FROM_NCOLS;
    nvec = (nbytes + (offs & (ssize - 1)) + ssize - 1) / ssize;

    /* add 7 to allow for small boot sectors */
    nvec += 7;

    /* construct the flash programming vector */
    v = vec = alloca (nvec * sizeof (vec[0]));
    for (o = offs, nb = nbytes; nb != 0;) {
	unsigned int soffs;
	unsigned int n;

	/* find current sector */
	if (sectorinfo (fcp, o, &soffs, &ssize)) {
	    ret = FLASHDEV_PROTECTED;
	    break;
	}

	/* work out number of bytes that we want to program in this sector */
	n = ssize - (o - soffs);
	if (n > nb)
	    n = nb;

	/* add to vector */
	assert (v < &vec[nvec]);
	v->offs = o;
	v->nb = n;
	v->mem = mem;
	v->scratch = 0;

	/* decide which columns need erasing */
	v->erase = fromerasemask (fcp, v, flags);

	/* ATMEL devices must reprogram whole sector */
	if (n != ssize && (v->erase || (fcp->flags & FROMFLG_ATMEL))) {
#ifdef ITROM
	    ret = FLASHDEV_PARTIAL;
	    break;
#else
	    /* a partial sector needs erasing - merge flash and data */
	    unsigned char *sbuf;
	    if (!(flags & FLASHDEV_PROG_MERGE)) {
		ret = FLASHDEV_PARTIAL;
		break;
	    }
	    sbuf = malloc (ssize);
	    if (!sbuf) {
		ret = FLASHDEV_NOMEM;
		break;
	    }
	    DBG("merging 0x%x-0x%x into sector at 0x%x-0x%x\n",
		o, o+n, soffs, soffs + ssize);
	    /* get sector from flash */
	    fromreadbytes (&fcp->common, soffs, sbuf, ssize, flags);
	    /* merge in new data */
	    memcpy (&sbuf[o - soffs], mem, n);
	    /* point vector at whole sector */
	    v->offs = soffs;
	    v->nb = ssize;
	    v->mem = sbuf;
	    v->scratch = 1;
	    /* calculate new column erase mask */
	    v->erase = fromerasemask (fcp, v, flags);
#endif
	}

	/* wipe this area of the flash out of the caches */
	fromcleancache (fcp, v, flags);

	/* advance to next sector */
	v++; nv++;
	mem += n;
	o += n;
	nb -= n;
    }

    if (ret == FLASHDEV_OK)
	/* now really do the programming operation */
	ret = fromprogramvec (fcp, vec, nv, flags);

    /* free any merged sector buffers */
#ifndef ITROM
    for (v = vec; nv != 0; v++, nv--)
	if (v->scratch)
	    free ((char *)v->mem);
#endif

    return ret;
}



/*
 * A simple one entry cache to optimise small sequential updates.
 */

#ifndef FROM_TINY
#define FROM_TINY FROM_LINESZ
#endif

#if FROM_TINY > 1

static unsigned char		tinyBuffer[FROM_TINY];
static fromcookie_t 		tinyCookie;
static unsigned int		tinyOffset;
static unsigned int		tinyFlags;

static int
fromprogramtiny (fromcookie_t fcp, unsigned int offs, 
	  const char *mem, unsigned int nb, int flags)
{
    int stat = FLASHDEV_OK;

    flags &= ~FLASHDEV_PROG_REBOOT;

    do {
	unsigned int ho = offs & ~(FROM_TINY - 1);
	unsigned int bo = offs & (FROM_TINY - 1);
	unsigned int n;

	n = nb;
	if (n > FROM_TINY - bo)
	    n = FROM_TINY - bo;

	if (tinyCookie && (tinyCookie != fcp || ho != tinyOffset)) {
	    /* flush old data from tiny buffer to flash */
	    DBG ("flush tiny cache to 0x%x\n", tinyOffset);
	    stat = fromprogramaligned (tinyCookie, tinyOffset, tinyBuffer, 
				       FROM_TINY, tinyFlags);
	    tinyCookie = NULL;
	}

	if (!mem)
	    break;	    /* a flush */

	if (!tinyCookie && stat == FLASHDEV_OK) {
	    /* fill tiny buffer from flash */
	    DBG ("fill tiny cache from 0x%x\n", ho);
	    stat = fromreadbytes (&fcp->common, ho, tinyBuffer, 
				  FROM_TINY, flags);
	    if (stat == FLASHDEV_OK) {
		tinyCookie = fcp;
		tinyOffset = ho;
		tinyFlags = flags;
	    }
	}

	if (n > 0 && stat == FLASHDEV_OK) {
	    /* write new data into tiny buffer */
	    DBG ("write %d bytes to tiny cache offs 0x%x\n", n, offs - ho);
	    memcpy (&tinyBuffer[offs - ho], mem, n);
	}

	mem += n;
	offs += n;
	nb -= n;
    } while (nb != 0 && stat == FLASHDEV_OK);

    return stat;
}
#endif



/*
 * The user callable programming routine
 */
static int
fromprogrambytes (flashcookie_t cp, unsigned int offs, 
		  const void *mem, unsigned int nb, int flags)
{
    fromcookie_t fcp = (fromcookie_t) cp;
    const unsigned char *m = mem;
    char scr[FROM_LINESZ];
    int stat = FLASHDEV_OK;

    /* check limits */
    if (offs + nb > fcp->size)
	return FLASHDEV_FAIL;

#if FROM_TINY > 1
    if (nb < FROM_TINY)
	/* a tiny transfer - use line cache */
	return fromprogramtiny (fcp, offs, m, nb, flags);
    
    /* else flush tiny cache */
    (void) fromprogramtiny (0, 0, 0, 0, 0);
#else
    if (nb == 0)
	return FLASHDEV_OK;
#endif

    /* handle leading unaligned bytes */
    if (offs & FROM_LINEMASK) {
	unsigned int lo = offs & ~FROM_LINEMASK;
	unsigned int bo = offs & FROM_LINEMASK;
	unsigned int n = FROM_LINESZ - bo;
	if (n > nb)
	    n = nb;

	DBG ("put %d bytes at end of line @ 0x%x\n", n, lo);
	stat = fromreadbytes (&fcp->common, lo, scr, FROM_LINESZ, flags);
	memcpy (scr + bo, m, n);
	if (stat == FLASHDEV_OK)
	    stat = fromprogramaligned (fcp, lo, scr, FROM_LINESZ, 
				       flags & ~FLASHDEV_PROG_REBOOT);

	offs += n;
	nb -= n;
	m += n;
    }

    /* handle trailing unaligned bytes */
    if ((nb & FROM_LINEMASK) && stat == FLASHDEV_OK) {
	unsigned int lo = offs + (nb & ~FROM_LINEMASK);
	unsigned int n = nb & FROM_LINEMASK;
	DBG ("put %d bytes to start of line @ 0x%x\n", n, lo);
	stat = fromreadbytes (&fcp->common, lo, scr, FROM_LINESZ, flags);
	memcpy (scr, m + nb - n, n);
	if (stat == FLASHDEV_OK)
	    stat = fromprogramaligned (fcp, lo, scr, FROM_LINESZ, 
				       flags & ~FLASHDEV_PROG_REBOOT);
	nb -= n;
    }

    /* do aligned central part */
    if (nb > 0 && stat == FLASHDEV_OK) {
	DBG ("program %d aligned bytes to @ 0x%x\n", nb, offs);
	stat = fromprogramaligned (fcp, offs, m, nb, flags);
    }

    return stat;
}



#ifdef FLASHROM_INLINE
static
#endif
int
_flashrom_probe (struct fromcookie *fcp, unsigned int offs)
{
#if FROM_WIDTH == 1
    const struct fromdevinfo *lastdv = NULL;
#endif
    const struct fromdevinfo *dv;
    const struct frommaninfo *mf;
    unsigned char man, dev;

    /* device size doesn't matter too much here */
    if (fcp->devsize == 0) {
	fcp->devsize 		= 0x100000;
	fcp->devsize_log2	= 20;
    }

#if FROM_WIDTH > 1
    /* get manufacture and device id */
    fcp->cmd1_offs = 0x5555;
    fcp->cmd2_offs = 0x2aaa;
    fcp->flags = FROMFLG_ATMEL | FROMFLG_DELAY;
    (void) fromautoselect (fcp, offs, &man, &dev);
#endif

    /* scan device type table for match */
    for (dv = devinfo; dv->name; dv++) {
	
#if FROM_WIDTH == 1
	if (!lastdv || ((lastdv->flags ^ dv->flags) & FROMFLG_BIWIDE)) {
	    /* get manufacture and device id */
	    if (dv->flags & FROMFLG_BIWIDE) {
		fcp->cmd1_offs = 0x2aaa;
		fcp->cmd2_offs = 0x5555;
	    } 
	    else {
		fcp->cmd1_offs = 0x5555;
		fcp->cmd2_offs = 0x2aaa;
	    }
	    fcp->flags = dv->flags | FROMFLG_ATMEL | FROMFLG_DELAY;
	    (void) fromautoselect (fcp, offs, &man, &dev);
	    lastdv = dv;
	}
#endif

	/* check for match - note special case for AMD/Fuji */
	if ((dv->man == man || (dv->man == MAN_AMD && man == MAN_FUJITSU))
	    && dv->dev == dev)
	    /* found it */
	    break;
    }

    if (!dv->name)
	/* unknown or missing device */
	return FLASHDEV_FAIL;

    /* find manufacture name */
    for (mf = manuf; mf->name && mf->id != man; mf++)
	continue;

    fcp->mf 		= mf;
    fcp->dv		= dv;
    fcp->flags		= dv->flags;
    fcp->secsize	= dv->secsize;
    fcp->devsize	= dv->dsize;
    fcp->mapbase	= FROM_MAPPABLE ? KVA_TO_PA (fcp->rbase) : ~0;
    fcp->common.dev 	= &fromdev;

    /* compute log2(device size) */
    for (fcp->devsize_log2 = 16; fcp->devsize != (1 << fcp->devsize_log2);
	 fcp->devsize_log2++)
	continue;

#if 0
    if (fcp->flags & FROMFLG_AM29DL) {
	fcp->dlbanksz[!(dv->boot == FBBOT)] = dv->dlbank1 * FROM_NCOLS;
	fcp->dlbanksz[(dv->boot == FBBOT)] = ((dv->dsize - dv->dlbank1) 
					      * FROM_NCOLS);
    }
#endif

    /* compute log2(device size) */
    return FLASHDEV_OK;
}
