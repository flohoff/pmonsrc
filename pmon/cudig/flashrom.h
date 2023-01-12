/*
 * flashrom.h: generic Flash ROM definitions
 * Copyright (c) 1999 Algorithmics Ltd.
 */


/* 
 * Boot sector types 
 */
enum frombootsects {FBNONE, FBTOP, FBBOT};

/* 
 * Device-specific flag bits 
 */
#if FROM_WIDTH == 1
#define FROMFLG_BIWIDE		0x0001	/* byte/word wide device */
#else
#define FROMFLG_BIWIDE		0	/* optimise away */
#endif

#ifndef FROM_NEVER_ATMEL
#define FROMFLG_DELAY		0x0002	/* 10ms delay after each cmd */
#define FROMFLG_ATMEL		0x0004	/* Atmel page based programming */
#else
#define FROMFLG_DELAY		0	/* optimise away */
#define FROMFLG_ATMEL		0	/* optimise away */
#endif

#ifndef FROM_NEVER_INTEL
#define FROMFLG_IBCS		0x0008	/* Intel basic command set */
#define FROMFLG_ISCS		0x0010	/* Intel scaleable command set */
#else
#define FROMFLG_IBCS		0	/* optimise away */
#define FROMFLG_ISCS		0	/* optimise away */
#endif

#ifndef FROM_NEVER_AMD
#define FROMFLG_AMD		0x0020	/* AMD style programming */
#define FROMFLG_BPASS		0x0040	/* AMD "unlock bypass" available */
#else
#define FROMFLG_AMD		0	/* optimise away */
#define FROMFLG_BPASS		0	/* optimise away */
#endif

#define FROMFLG_PROT		0x0080	/* "sector protect" available */

/* 
 * Family variants
 */

/* Byte Mode only */
#define TYPE_aF004	FROMFLG_AMD|FROMFLG_PROT
#define TYPE_aF040	FROMFLG_AMD|FROMFLG_PROT
#define TYPE_iF004	FROMFLG_IBCS
#define TYPE_iF040	FROMFLG_IBCS
#define TYPE_tC040	FROMFLG_ATMEL|FROMFLG_DELAY

/* Byte/Word switchable */
#define TYPE_aF400	FROMFLG_BIWIDE|FROMFLG_AMD|FROMFLG_PROT
#define TYPE_LV400	FROMFLG_BIWIDE|FROMFLG_AMD|FROMFLG_BPASS|FROMFLG_PROT
#define TYPE_iF400	FROMFLG_BIWIDE|FROMFLG_IBCS
#define TYPE_sF008	FROMFLG_BIWIDE|FROMFLG_IBCS|FROMFLG_PROT
#define TYPE_iF160	FROMFLG_BIWIDE|FROMFLG_IBCS|FROMFLG_ISCS|FROMFLG_PROT

/* 
 * Command codes 
 */

/* Generic */
#define FROMCMD_AUTOSELECT 	0x90

/* AMD/Atmel */
#define FROMCMD_AMD_ERASECHIP	0x10
#define FROMCMD_AMD_BYPASS	0x20
#define FROMCMD_AMD_ERASESECT	0x30
#define FROMCMD_AMD_ERASE	0x80
#define FROMCMD_AMD_BYPASS_RST	0x90
#define FROMCMD_AMD_PROGRAM	0xa0
#define FROMCMD_AMD_RESET	0xf0

/* Intel BCS */
#define FROMCMD_IBCS_ERASE	0x20
#define FROMCMD_IBCS_PROGRAM	0x40
#define FROMCMD_IBCS_CLEARSR	0x50
#define FROMCMD_IBCS_READSR	0x70
#define FROMCMD_IBCS_ERASESUSP	0xb0
#define FROMCMD_IBCS_ERASECONF	0xd0
#define FROMCMD_IBCS_RESET	0xff

/* Manufacturer codes returned from AUTOSELECT */
#define MAN_AMD			0x01
#define MAN_FUJITSU		0x04
#define MAN_ATMEL		0x1f
#define MAN_MICRON		0x2c
#define MAN_INTEL		0x89
#define MAN_SHARP		0xb0

/* AMD magic values to unlock device */
#define AMD_CMD1_DATA		0xaa
#define AMD_CMD2_DATA		0x55

/* Data bit definitions for AMD "embedded programming algorithm" */
#define AMD_DQPOLL		0x80
#define AMD_DQTOGGLE		0x40
#define AMD_DQTIMEEXCEEDED	0x20
#define AMD_DQSECTERASETIMER 	0x08

/* Data bit definitions for IBCS status register */
#define ICSR_READY		0x80
#define ICSR_ERASE_SUSP		0x40
#define ICSR_ERASE_ERR		0x20
#define ICSR_WRITE_ERR 		0x10
#define ICSR_VPP_ERR 		0x08


/* Manufacturer-specific information */
struct frommaninfo {
    const char *	name;		/* manufacturer name */
    unsigned char	id;		/* manufacturer id */
};


/* Device-specific information */
struct fromdevinfo {
    const char *	name;			/* device name */
    unsigned char	man;			/* manufacturer id */
    unsigned char	dev;			/* device id */
    enum frombootsects	boot:8;			/* boot block type */
    unsigned int	dsize;			/* device size in bytes */
    unsigned int	secsize;		/* base sector size in bytes */
    unsigned int	flags;			/* device specific flags */
};


#if FROM_WIDTH == 1
typedef unsigned char	from_t;
#elif FROM_WIDTH == 2
typedef unsigned short	from_t;
#else
#error Bad value for FROM_WIDTH
#endif

#if (FROM_GROUP * FROM_WIDTH) == 1
typedef unsigned char	fromgroup_t;
#elif (FROM_GROUP * FROM_WIDTH) == 2
typedef unsigned short	fromgroup_t;
#elif (FROM_GROUP * FROM_WIDTH) == 4
typedef unsigned long	fromgroup_t;
#elif (FROM_GROUP * FROM_WIDTH) == 8
typedef unsigned long long fromgroup_t;
#else
#error Bad value for FROM_GROUP
#endif

#if FROM_GROUP == 1
#define fromgroup(x) \
  ((from_t)(x))
#elif FROM_GROUP == 2
#define fromgroup(x) \
 ({fromgroup_t __x = (from_t)(x); \
   __x |= __x << (FROM_WIDTH * 8); \
 })
#elif FROM_GROUP == 4
#define fromgroup(x) \
 ({fromgroup_t __x = (from_t)(x); \
   __x |= __x << (FROM_WIDTH * 8); \
   __x |= __x << (FROM_WIDTH * 8 * 2); \
 })
#endif
	
#if FROM_GROUP == 1
#define foreach_word(word)	word = 0;
#else
#define foreach_word(word)	for (word = 0; word < FROM_GROUP; word++)
#endif

#if FROM_GROUP == FROM_NCOLS
#define foreach_group(col)	col = 0;
#else
#define foreach_group(col)	for (col = 0; col < FROM_NCOLS; col += FROM_GROUP)
#endif

/* Internal handle for device */
struct fromcookie {
    struct flashcookie		common;
    from_t			*rbase;		/* read address */
    volatile from_t		*pbase;		/* programming address */
    const struct fromdevinfo	*dv;		/* device-specific info */
    const struct frommaninfo	*mf;		/* manufacturer info */
    /* safe copies of device-specific info */
    unsigned int		devsize;	/* device size */
    unsigned int		secsize; 	/* base sector size */
    unsigned int		flags;		/* device specific flags */
    unsigned short		cmd1_offs;	/* AMD command offs #1 */
    unsigned short		cmd2_offs;	/* AMD command offs #2 */
    /* board-specific info */
    unsigned char		devsize_log2;	/* log2(devsize) */
    unsigned char		nbanks;		/* # sequential banks */
};

typedef const struct fromcookie *fromcookie_t;

extern int _flashrom_find (struct fromcookie *, unsigned int);
