/* $Id: pmon.h,v 1.22 2001/09/04 17:40:07 chris Exp $ */
/* pmon.h - The part of debugger for MIPS CPUs
 *
 * PMON Written for LSI LOGIC Corp. by Phil Bunce.
 *              Phil Bunce, 100 Dolores St. Ste 242, Carmel CA 93923
 */

#ifndef __PMON_H__
#define __PMON_H__

#ifndef _MIPS_
#include "mips.h"
#endif

/* default start address for client */
#ifndef CLIENTPC
#define CLIENTPC 0x80020000
#endif

/* flush_cache types */
#define ICACHE  0
#define DCACHE  1
#define IADDR   2

#ifdef SABLE
#define TIKRATE 256
#else
#define TIKRATE 256000
#endif

#ifdef LANGUAGE_C
#define SBD_DISPLAY(msg, n) \
do {\
  extern void sbddisplay (unsigned long, unsigned int); \
  const unsigned char * const __m = (unsigned char *)(msg); \
  sbddisplay((__m[0]<<24) | (__m[1]<<16) | (__m[2]<<8) | __m[3], n); \
} while (0)
#else
#define SBD_DISPLAY(c0,c1,c2,c3, n) \
	li	a0,+((c0)<<24) | ((c1)<<16) | ((c2)<<8) | (c3); \
	li	a1,+(n); \
	bal	sbddisplay; \
	sll	zero,zero,0
#endif

#define CHKPNT_SBDR	0x01	/* in sbdreset */
#define CHKPNT_CACH	0x02	/* calling init_cache */
#define	CHKPNT_CODE	0x03	/* copying text area to ram */
#define	CHKPNT_DATA	0x04	/* copying data area to ram */
#define CHKPNT_ZBSS	0x05	/* clearing bss */
#define CHKPNT_HAND	0x06	/* installing exception handlers */
#define CHKPNT_SBDM	0x07	/* calling sbdmachinit */
#define CHKPNT_CREG	0x08	/* setting initial client conditions */
#define CHKPNT_DBGI	0x09	/* calling dbginit */
#define CHKPNT_MAIN	0x0a	/* calling main */
#define CHKPNT_EXIT	0x0b	/* calling exit */
#define CHKPNT_FREQ	0x0c	/* determining CPU frequency */
#define CHKPNT_0MEM	0x0d	/* no memory detected */
#define CHKPNT_WAIT	0x20	/* waiting for debugger connect */
#define CHKPNT_RDBG	0x21	/* entering remote debug mode */
#define CHKPNT_AUTO	0x22	/* waiting for autoboot timeout */
#define CHKPNT_ENVI	0x23	/* calling envinit */
#define CHKPNT_SBDD	0x24	/* calling sbddevinit */
#define CHKPNT_DEVI	0x25	/* calling devinit */
#define CHKPNT_NETI	0x26	/* calling init_net */
#define CHKPNT_HSTI	0x27	/* calling histinit */
#define CHKPNT_SYMI	0x28	/* calling syminit */
#define CHKPNT_DEMO	0x29	/* calling demoinit */
#define CHKPNT_MACH	0x2a	/* calling getmachtype */
#define CHKPNT_SBDE	0x2b	/* calling sbdenable */
#define CHKPNT_LOGO	0x2c	/* printing PMON signon */
#define CHKPNT_SHRC	0x2d	/* calling shrc */
#define CHKPNT_PMON	0x2e	/* set state */
#define CHKPNT_PRAM	0x2f	/* set state */
#define CHKPNT_RUN	0x30	/* set state */
#define CHKPNT_MAPV	0x31	/* loading non-volatile env */
#define CHKPNT_BRDV	0x32	/* setting board defaults */
#define CHKPNT_STDV	0x33	/* setting PMON env defaults */
#define CHKPNT_ITLB	0x34	/* calling init_tlb */

#define CHKPNT_BEV	0xee	/* boot exception */

#define CHKPNT_PCIH	0x40	/* calling pci_hwinit */
#define CHKPNT_PCIS	0x41	/* scanning PCI bus */
#define CHKPNT_PCIR	0x42	/* calling pci_hwreinit */
#define CHKPNT_PCIW	0x43	/* configuring PCI windows */
#define CHKPNT_PCID	0x44	/* calling pci_setup_devices */
 
#define MAX_BPT	 64		/* max number of user breakpoints */
#define LINESZ	200		/* max command line length */
#define MAX_AC	100		/* max number of args to commands */
#define PATSZ    80		/* max pattern length for search & fill */

#define NO_BPT		-1L
#define BPT_CODE	0x0000000dL	/* BREAK instruction */
#define SIGNATURE	0x4572696e

/* trace_mode states */
#define TRACE_NO	0	/* no trace/go */
#define TRACE_TB	1	/* trace & break */
#define TRACE_TG	2	/* trace & go */
#define TRACE_GB	3	/* go & break */
#define TRACE_DG	4	/* debug mode step and go */
#define TRACE_DC	5	/* debug mode continue */
#define TRACE_DS	6	/* debug mode step */
#define TRACE_TN	7	/* multistep trace */
#define TRACE_TW	8	/* trace watchpoint step */
#define TRACE_DW	9	/* debug mode watchpoint step */

/* regsize states */
#define REGSZ_32	0
#define REGSZ_64	1

#define CNTRL(x) (x & 0x1f)

#ifdef LANGUAGE_C
#define loop for (;;)

typedef char    byte;		/* 1 byte */
typedef short   half;		/* 2 bytes */
typedef long    word;		/* 4 bytes */
typedef long    vaddr;		/* 4 bytes */
#if __GNUC__ >= 2
typedef long long dword;	/* 8 bytes */
#endif

typedef unsigned char ubyte;
typedef unsigned short uhalf;
typedef unsigned long uword;
#if __GNUC__ >= 2
typedef unsigned long long udword;
#endif

#ifndef _RM7KC0_H_	/* rm7kc0.h also defines these typedefs.*/
#ifndef _REG_T_
#if __mips >= 3
typedef signed long long	sreg_t;
typedef unsigned long long	reg_t;
#else
typedef signed long 		sreg_t;
typedef unsigned long		reg_t;
#endif
#endif /* !_REG_T */
#endif /* _RM7KC0_H_ */

#if __mips >= 3
typedef unsigned long long	ureg_t;
#else
typedef unsigned long		ureg_t;
#endif

typedef int     Func ();

#if 0
#define MAXSYM 12
typedef struct Sym {
    struct Sym     *anext;
    struct Sym     *nnext;
    char           *lname;
    unsigned int    value;
    char            name[MAXSYM];
} Sym;
#endif

typedef struct Demo {
    const char     *name;
    Func           *addr;
} Demo;

typedef struct Optdesc {
    const char     *name;
    const char     *desc;
} Optdesc;

typedef struct Cmd {
    const char     *name;
    const char     *opts;
    const Optdesc  *optdesc;
    const char     *desc;
    Func           *func;
    int             minac;
    int             maxac;
    int             repeat;
} Cmd;

typedef struct Bps {
    unsigned long   addr;
    unsigned long   len;
    unsigned long   value;
    unsigned long   wpt;
    char           *cmdstr;
} Bps;

typedef struct Stopentry {
    unsigned long   addr;
    unsigned long   value;
    char            name[10];
    char            sense;
} Stopentry;

typedef enum {
    RD_RS_RT, RT_RS_IMM, OFF, RS_RT_OFF, RS_OFF, NONE, RT_RD, COFUN, RD_RT,
    RS_RT, TARGET, JALR, RS, RD_RT_SFT, LOAD_STORE, RT_IMM, RD, RD_RT_RS, RD_RS,
    RT_RS_SIMM, RS_SIMM, RT_SIMM, JR, RT_C0, RT_C1, RT_CN, RT_CC1, LDSTC0, LDSTC1,
    LDSTCN, WORD, RT_C2, RT_CC2, BPCODE, CACHE_OP, 
    CP_OFF, STORE, STOREC1, STORECN,
    FT_FS_FD_D, FT_FS_FD_S, FS_FD_D, FS_FD_S, FS_FD_W, FS_FD_L, FT_FS_D, FT_FS_S,
    RT_RD_TO, RT_CC1_TO, RT_CN_TO, RT_C0_TO, RT_C1_TO
} TYPE;

typedef struct {
    const char     *str;
    long            mask, code;
    TYPE            type;
} DISTBL;

typedef struct RegSpec {	/* register field info */
    byte            size;	/* size (width) of field */
    byte            lsb;	/* position of field's lsb */
    const char     *name;	/* field name */
    byte            base;	/* display base e.g. 2 8 10 16 */
    const char * const *values;	/* use this if base=0 */
    byte            ro;		/* read only */
    unsigned int    flags:24;
} RegSpec;

typedef struct RegList {	/* register info */
    reg_t          *reg;	/* address of register value */
    const RegSpec  *spec;	/* field info */
    const char     *name;	/* primary name */
    const char     *aname;	/* alternate name */
    byte           regnum;	/* register number */
    unsigned int   flags:24;
} RegList;

/* defs for RegList flags field */
#define F_ANAME 0x01		/* print aname */
#define F_GPR	0x02		/* gp reg */
#define F_CF	0x04		/* call function for value */
#define F_WO	0x08		/* write only */
#define F_RO	0x10		/* read only */
#define F_FPU	0x20		/* needs an fpu */
#define F_64	0x40		/* 64-bit register */
/* cpu types */
#define F_00	0x000100	/* LR33000 only */
#define F_20	0x000200	/* LR33020 only */
#define F_50	0x000400	/* LR33050 only */
#define F_3081	0x000800	/* R3081 only */
#define F_3041	0x001000	/* R3041 only */
#define F_4650	0x002000	/* in R4650/40 only */
#define F_5000	0x004000	/* in R5000 only */
#define F_4100	0x008000	/* in R4100 only */
#define F_7000	0x010000	/* in RM7000 only */
#define F_5400	0x020000	/* in Vr54xx only */
#define F_OTHER 0x800000	/* in all other types */ 
#define F_ALL	0xffff00

#define FS_(x)		(((x) >> 11) & ((1L <<  5) - 1))
#define FT_(x)		(((x) >> 16) & ((1L <<  5) - 1))
#define FD_(x)		(((x) >>  6) & ((1L <<  5) - 1))
#define RS_(x)		(((x) >> 21) & ((1L <<  5) - 1))
#define RT_(x)		(((x) >> 16) & ((1L <<  5) - 1))
#define RD_(x)		(((x) >> 11) & ((1L <<  5) - 1))
#define IMM_(x)		(((x) >>  0) & ((1L << 16) - 1))
#define TARGET_(x)	(((x) >>  0) & ((1L << 26) - 1))
#define SHAMT_(x)	(((x) >>  6) & ((1L <<  5) - 1))

#ifdef RTC
struct tm {
    int	tm_sec;		/* seconds after the minute [0-60] */
    int	tm_min;		/* minutes after the hour [0-59] */
    int	tm_hour;	/* hours since midnight [0-23] */
    int	tm_mday;	/* day of the month [1-31] */
    int	tm_mon;		/* months since January [0-11] */
    int	tm_year;	/* years since 1900 */
    int	tm_wday;	/* days since Sunday [0-6] */
    int	tm_yday;	/* days since January 1 [0-365] */
    int	tm_isdst;	/* Daylight Savings Time flag */
    long tm_gmtoff;	/* offset from UTC in seconds */
    char *tm_zone;	/* timezone abbreviation */
};

typedef long time_t;

time_t mktime (struct tm *);
time_t gmmktime (const struct tm *);
struct tm *gmtime (const time_t *);
struct tm *localtime (const time_t *);
time_t sbd_gettime (void);
void sbd_settime (time_t);
#endif

/* external data declarations */
extern const DISTBL   distbl[];
extern reg_t    DBGREG[];	/* debugger's register value holder */
extern int      histno;		/* current history number */
extern char    *searching;
extern const Cmd CmdTable[];
extern jmp_buf  jmpb;
extern char     line[LINESZ + 1];
extern int      trace_mode;
extern Bps      Bpt[];
extern Bps      BptTmp;
extern Bps      BptTrc;
extern Bps      BptTrcb;
extern char     date[];
extern char     vers[];
extern int      machtype;
extern unsigned long initial_sr;
extern char     prnbuf[];
extern int      memorysize;
extern unsigned int moresz;
extern int	repeating_cmd;

#if defined(FLOATINGPT) && !defined(NEWFP)
extern struct c1state *pmc1dat;		/* state info for FPU in PMON */
extern struct c1state *clic1dat;	/* state info for FPU in client */
#endif

/* function declarations */
#ifdef __STDC__
const DISTBL   *get_distbl (uword);
void	       *malloc (unsigned long);
void	       *realloc (void *, unsigned long);
char           *getenv (const char *);
char           *findbang (char *);
char           *gethistn (int);
char           *getexcname (int);
unsigned long   getpchist (int);
word            dispmem (char *, word, int);
long            disasm (char *, long, long);
int		disp_as_reg (reg_t *, char *, int *);
int		sym2adr (word *, char *);
char 		*adr2sym (char *, unsigned long);
int		getreg (sreg_t *, char *);
int		getregadr (sreg_t **, char *);
int		get_rsa (uword *, char *);
int		get_rsa_reg (ureg_t *, char *);
int		is_branch (word);
int		is_jal (word);
int		atob (unsigned long *, char *, int);
int		llatob (unsigned long long *, char *, int);
int             cXc2 (int, int, int), mXc2 (int, int, int);
int             mXc0 (int, int, int);
void		store_dword (word, dword);
void		store_word (word, word);
void		store_half (word, half);
void		store_byte (word, byte);
#else
const DISTBL   *get_distbl ();
byte           *scan_byte ();
void	       *malloc ();
Sym            *findvalue ();
char           *getenv ();
char           *findbang ();
char           *gethistn ();
char           *getexcname ();
unsigned long   getpchist ();
word            dispmem ();
long            disasm ();
int             cXc2 (), mXc2 ();
int             mXc0 ();
void		store_dword ();
void		store_word ();
void		store_half ();
void		store_byte ();
#endif

#if __mips >= 3
#define ator	llatob
#else
#define ator	atob
#endif

#define cmpstr(x,y)	((strcmp(x,y) == 0)?1:0)
#define getfield(w,s,p)	((((unsigned long)w)&(((1<<s)-1)<<p))>>p)
#define load_byte(adr)	(*(byte *)(adr))
#define load_half(adr)	(*(half *)(adr))
#define load_word(adr)	(*(word *)(adr))
#define load_dword(adr)	(*(dword *)(adr))
#define load_reg(adr)	(*(sreg_t *)(adr))

/* macros to increment and decrement x, modulus mod */
#define incmod(x,mod)	(((x+1) > mod)?0:x+1)
#define decmod(x,mod)	(((x-1) < 0)?mod:x-1)

#define Gpr		(&DBGREG[R_ZERO])
#define Hi		DBGREG[R_HI]
#define Lo		DBGREG[R_LO]
#define Epc		DBGREG[R_EPC]
#define Status		DBGREG[R_STATUS]
#define Cause		DBGREG[R_CAUSE]
#define Badva		DBGREG[R_BADVA]
#define Prid		DBGREG[R_PRID]

#ifdef FLOATINGPT
#define Fpr		(&DBGREG[R_F0])
#define Fcr		DBGREG[R_FCR]
#define Fid		DBGREG[R_FID]
#endif

#ifdef LR33000
#define Dcic		DBGREG[R_DCIC]
#define Bpc		DBGREG[R_BPC]
#define Bda		DBGREG[R_BDA]
#else
#define Context		DBGREG[R_CONTEXT]
#define Entryhi		DBGREG[R_ENTRYHI]
#define Entrylo		DBGREG[R_ENTRYLO]
#define Index		DBGREG[R_INDEX]
#define Random		DBGREG[R_RANDOM]
#ifdef R4000
#define Entrylo0	DBGREG[R_ENTRYLO0]
#define Entrylo1	DBGREG[R_ENTRYLO1]
#define PgMask		DBGREG[R_PGMASK]
#define WatchLo		DBGREG[R_WATCHLO]
#define WatchHi		DBGREG[R_WATCHHI]
#define Wired		DBGREG[R_WIRED]
#define Config		DBGREG[R_CONFIG]
#define LLAddr		DBGREG[R_LLADDR]
#define Ecc		DBGREG[R_ECC]
#define CacheErr	DBGREG[R_CACHERR]
#define TagLo		DBGREG[R_TAGLO]
#define TagHi		DBGREG[R_TAGHI]
#define ErrEpc		DBGREG[R_ERREPC]
#define XContext	DBGREG[R_XCONTEXT]
#define WatchMask	DBGREG[R_WATCHMASK]
#define Icr		DBGREG[R_ICR]
#define IplLo		DBGREG[R_IPLLO]
#define IplHi		DBGREG[R_IPLHI]
#define Info		DBGREG[R_INFO]
#endif
#endif

#endif /* LANGUAGE_C */


/* Target registers index */
#define	R_ZERO		0
#define	R_AT		1
#define	R_V0		2
#define	R_V1		3
#define	R_A0		4
#define	R_A1		5
#define	R_A2		6
#define	R_A3		7
#define	R_T0		8
#define	R_T1		9
#define	R_T2		10
#define	R_T3		11
#define	R_T4		12
#define	R_T5		13
#define	R_T6		14
#define	R_T7		15
#define	R_S0		16
#define	R_S1		17
#define	R_S2		18
#define	R_S3		19
#define	R_S4		20
#define	R_S5		21
#define	R_S6		22
#define	R_S7		23
#define	R_T8		24
#define	R_T9		25
#define	R_K0		26
#define	R_K1		27
#define	R_GP		28
#define	R_SP		29
#define	R_FP		30
#define	R_RA		31
#define R_HI		32
#define R_LO		33

#ifdef LR33000
#define R_DCIC		34
#define R_BPC		35
#define R_BDA		36
#else
#define R_ENTRYHI	34
#define R_ENTRYLO	35
#define R_ENTRYLO0	35
#define R_INDEX		36
#define R_RANDOM	37
#endif

#define R_STATUS	38
#define R_CAUSE		39
#define R_EPC		40
#ifndef LR33000
#define R_CONTEXT	41
#define R_BADVA		42
#endif
#define R_PRID		43
#define R_K1TMP		44

#if defined(R4000)
#define R_ENTRYLO1	45
#define R_PGMASK	46
#define R_WIRED		47
#define R_CONFIG	48
#define R_LLADDR	49
#define R_WATCHLO	50
#define R_WATCHHI	51
#define R_ECC		52
#define R_CACHERR	53
#define R_TAGLO		54
#define R_TAGHI		55
#define R_ERREPC	56
#define R_XCONTEXT	57
#define R_WATCHMASK	58
#define R_ICR		59
#define R_IPLLO		60
#define R_IPLHI		61
#define R_INFO		62
#define NONFPREGS	63
#else
#define NONFPREGS	45
#endif

#ifdef FLOATINGPT
#define R_F0		NONFPREGS
#define R_F1		(R_F0+1)
#define R_F2		(R_F0+2)
#define R_F3		(R_F0+3)
#define R_F4		(R_F0+4)
#define R_F5		(R_F0+5)
#define R_F6		(R_F0+6)
#define R_F7		(R_F0+7)
#define R_F8		(R_F0+8)
#define R_F9		(R_F0+9)
#define R_F10		(R_F0+10)
#define R_F11		(R_F0+11)
#define R_F12		(R_F0+12)
#define R_F13		(R_F0+13)
#define R_F14		(R_F0+14)
#define R_F15		(R_F0+15)
#define R_F16		(R_F0+16)
#define R_F17		(R_F0+17)
#define R_F18		(R_F0+18)
#define R_F19		(R_F0+19)
#define R_F20		(R_F0+20)
#define R_F21		(R_F0+21)
#define R_F22		(R_F0+22)
#define R_F23		(R_F0+23)
#define R_F24		(R_F0+24)
#define R_F25		(R_F0+25)
#define R_F26		(R_F0+26)
#define R_F27		(R_F0+27)
#define R_F28		(R_F0+28)
#define R_F29		(R_F0+29)
#define R_F30		(R_F0+30)
#define R_F31		(R_F0+31)
#define R_FCR		(R_F0+32)
#define R_FID		(R_F0+33)
#endif

#ifdef FLOATINGPT 
#define NREGS		(R_FID + 1)
#else
#define NREGS		NONFPREGS
#endif

#ifdef R4000
#ifndef __ASSEMBLER__
struct tlbentry {
    reg_t	tlb_mask;
    reg_t	tlb_hi;
    reg_t	tlb_lo0;
    reg_t	tlb_lo1;
};
#endif
#if __mips >= 3
#define TLB_MASK	(0*8)
#define TLB_HI		(1*8)
#define TLB_LO0		(2*8)
#define TLB_LO1		(3*8)
#else
#define TLB_MASK	(0*4)
#define TLB_HI		(1*4)
#define TLB_LO0		(2*4)
#define TLB_LO1		(3*4)
#endif

#else
#ifndef __ASSEMBLER__
struct tlbentry {
    unsigned int tlb_hi;
    unsigned int tlb_lo;
};
#endif
#define TLB_HI		(0*4)
#define TLB_LO		(1*4)
#endif

#ifdef R4000
#define CERR_FIFO_SIZE	32
#define CERR_FIFO_MASK	(CERR_FIFO_SIZE-1)

#ifdef __ASSEMBLER__
	.struct	0
ce_in:	.word	0
ce_out:	.word	0
ce_buf:	.space	CERR_FIFO_SIZE*4
	.previous
#else
struct cerrfifo {
    unsigned int	ce_in, ce_out;
    unsigned int	ce_buf[CERR_FIFO_SIZE];
};

extern struct cerrfifo	cerrfifo;

#endif
#endif /* R4000 */


#if defined( LANGUAGE_C )
#ifdef MIPSEL
struct IEEEdp {
    unsigned manl:32;
    unsigned manh:20;
    unsigned exp:11;
    unsigned sign:1;
};

struct IEEEsp {
    unsigned man:23;
    unsigned exp:8;
    unsigned sign:1;
};
#else
struct IEEEdp {
    unsigned sign:1;
    unsigned exp:11;
    unsigned manh:20;
    unsigned manl:32;
};

struct IEEEsp {
    unsigned sign:1;
    unsigned exp:8;
    unsigned man:23;
};
#endif
int spdenorm( struct IEEEsp *  );
int dpdenorm( struct IEEEdp * );
int spnan( struct IEEEsp *  );
int dpnan( struct IEEEdp * );
#endif

#endif /* __PMON_H__ */
