/* $Id: ecoff.h,v 1.3 1997/09/08 15:29:19 nigel Exp $ */

/* Standard COFF File Header */
struct filehdr {
    unsigned short	f_magic;	/* magic number */
    unsigned short	f_nscns;	/* number of sections */
    long		f_timdat;	/* time & date stamp */
    long		f_symptr;	/* file pointer to symbolic header */
    long		f_nsyms;	/* sizeof(symbolic hdr) */
    unsigned short	f_opthdr;	/* sizeof(optional hdr) */
    unsigned short	f_flags;	/* flags */
};
#define	FILHSZ	sizeof(struct filehdr)

#define MIPSEBMAGIC	 0x160
#define MIPSELMAGIC	 0x162

#define MIPS2EBMAGIC	 0x163
#define MIPS2ELMAGIC	 0x166

#define MIPS3EBMAGIC	 0x140
#define MIPS3ELMAGIC	 0x142

#include <machine/endian.h>
#if BYTE_ORDER==BIG_ENDIAN
#define MIPSMAGIC	MIPSEBMAGIC
#define MIPS2MAGIC	MIPS2EBMAGIC
#define MIPS3MAGIC	MIPS3EBMAGIC
#else
#define MIPSMAGIC	MIPSELMAGIC
#define MIPS2MAGIC	MIPS2ELMAGIC
#define MIPS3MAGIC	MIPS3ELMAGIC
#endif

#define MIPS1_OK(x) ((x).f_magic == MIPSMAGIC)

#define MIPS2_OK(x) ((x).f_magic == MIPS2MAGIC)

#define MIPS3_OK(x) ((x).f_magic == MIPS3MAGIC)

#if CPU_R3000
#define MIPS_OK(x) MIPS1_OK(x)
#elif CPU_R4000
#define MIPS_OK(x) (MIPS1_OK(x) || MIPS2_OK(x) || MIPS3_OK(x))
#else
#define MIPS_OK(x) 0
#endif

/* MIPS a.out header */
struct aouthdr {
    short	magic;
    short	vstamp;		/* version stamp */
    long	tsize;		/* text size */
    long	dsize;		/* data size */
    long	bsize;		/* bss size */
    long	entry;		/* entry point */
    long	text_start;	/* text base address */
    long	data_start;	/* data base address */
    long	bss_start;	/* bss base address */
    long	dummy[6];
};
#define AOUTHSZ	sizeof(struct aouthdr)

#define	OMAGIC		0x0107
#define	NMAGIC		0x0108
#define	ZMAGIC		0x010b

#define	SCNHSZ		40

#define N_TXTOFF(f, a) \
 ((a).magic == ZMAGIC ? 0 : ((a).vstamp < 23 ? \
   ((FILHSZ + AOUTHSZ + (f).f_nscns * SCNHSZ + 7) & ~7) : \
   ((FILHSZ + AOUTHSZ + (f).f_nscns * SCNHSZ + 15) & ~15) ) )

#define	N_BADMAG(x) \
    (((x).magic)!=OMAGIC && ((x).magic)!=NMAGIC && ((x).magic)!=ZMAGIC)
