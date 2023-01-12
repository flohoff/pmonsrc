/*
 * flashrom.h: generic Flash ROM definitions
 * Copyright (c) 1999 Algorithmics Ltd.
 */

#ifndef __FLASHROM_H__
#define __FLASHROM_H__

/* default to all flash families disabled */
#ifndef FROM_FAMILY_ATMEL
#define FROM_FAMILY_ATMEL 	0
#endif
#ifndef FROM_FAMILY_AM29DL
#define FROM_FAMILY_AM29DL 	0
#endif
#ifndef FROM_FAMILY_AMD
#define FROM_FAMILY_AMD		(FROM_FAMILY_AM29DL|FROM_FAMILY_ATMEL)
#endif
#ifndef FROM_FAMILY_INTEL
#define FROM_FAMILY_INTEL 	0
#endif

#if (FROM_FAMILY_AMD+FROM_FAMILY_AM29DL+FROM_FAMILY_ATMEL+FROM_FAMILY_INTEL)==0
# error no flashrom families defined
#endif

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
#define FROMFLG_BIWIDE		0
#endif

#if FROM_FAMILY_ATMEL
#define FROMFLG_DELAY		0x0002	/* 10ms delay after each cmd */
#define FROMFLG_ATMEL		0x0004	/* Atmel page based programming */
#else
#define FROMFLG_DELAY		0
#define FROMFLG_ATMEL		0
#endif

#if FROM_FAMILY_INTEL
#define FROMFLG_IBCS		0x0008	/* Intel basic command set */
#define FROMFLG_ISCS		0x0010	/* Intel scaleable command set */
#else
#define FROMFLG_IBCS		0
#define FROMFLG_ISCS		0
#endif

#if FROM_FAMILY_AMD
#define FROMFLG_AMD		0x0020	/* AMD style programming */
#define FROMFLG_BPASS		0x0040	/* AMD "unlock bypass" available */
#else
#define FROMFLG_AMD		0
#define FROMFLG_BPASS		0
#endif

#if FROM_FAMILY_AM29DL
#define FROMFLG_AM29DL		0x0080	/* AMD 29DL asymettric banks */
#else
#define FROMFLG_AM29DL		0
#endif

#define FROMFLG_PROT		0x0100	/* "sector protect" available */

#if FROM_FAMILY_INTEL
#define FROMFLG_SPROT		0x0200	/* sharp "sector protect" available */
#else
#define FROMFLG_SPROT		0
#endif

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
#define TYPE_LV400	(TYPE_aF400|FROMFLG_BPASS)
#define TYPE_aDL	(TYPE_LV400|FROMFLG_AM29DL)
#define TYPE_iF400	FROMFLG_BIWIDE|FROMFLG_IBCS
#define TYPE_s008C	(TYPE_iF400|FROMFLG_PROT)
#define TYPE_s016U	(TYPE_iF400|FROMFLG_SPROT)
#define TYPE_iF160	(TYPE_iF400|FROMFLG_ISCS|FROMFLG_PROT)

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
#define FROMCMD_IBCS_CONFIRM	0xd0
#define FROMCMD_IBCS_RESET	0xff

/* Sharp BCS extensions */
#define FROMCMD_SHARP_RXSR	0x71
#define FROMCMD_SHARP_SWAPPB	0x72
#define FROMCMD_SHARP_LOADPB	0x74
#define FROMCMD_SHARP_READPB	0x75
#define FROMCMD_SHARP_SEQLOADPB	0xe0
#define FROMCMD_SHARP_WRITEPB	0x0c
#define FROMCMD_SHARP_LOCKBLOCK	0x77
#define FROMCMD_SHARP_UPSTAT 	0x97
#define FROMCMD_SHARP_UPINFO 	0x99
#define FROMCMD_SHARP_ERASEALL	0xa7
#define FROMCMD_SHARP_RDYCTL	0x96
#define FROMCMD_SHARP_RDY_LEVEL	 	0x01
#define FROMCMD_SHARP_RDY_PULSE_W	0x02
#define FROMCMD_SHARP_RDY_PULSE_E	0x03
#define FROMCMD_SHARP_RDY_OFF		0x04
#define FROMCMD_SHARP_SLEEP	0xf0
#define FROMCMD_SHARP_ABORT	0x80


/* Manufacturer codes returned from AUTOSELECT */
#define MAN_AMD			0x01
#define MAN_FUJITSU		0x04
#define MAN_ATMEL		0x1f
#define MAN_ST			0x20
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

/* Data bit definitions for Sharp global status register */
#define SGSR_READY		0x80
#define SGSR_SUSP		0x40
#define SGSR_ERR		0x20
#define SGSR_SLEEP		0x10
#define SGSR_QFULL		0x08
#define SGSR_PBAVAIL		0x04
#define SGSR_PBREADY		0x02
#define SGSR_PBSELECT		0x01

/* Data bit definitions for Sharp block status register */
#define SBSR_READY		0x80
#define SBSR_UNLOCK		0x40
#define SBSR_ERR		0x20
#define SBSR_ABORT		0x10
#define SBSR_QFULL		0x08
#define SBSR_VPPLOW		0x04

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
    unsigned int	dlbank1;		/* AM29DL bank1 size */
};


typedef volatile char	*from_pt;

#if FROM_WIDTH == 1
typedef unsigned char	fromdev_t;
#elif FROM_WIDTH == 2
typedef unsigned short	fromdev_t;
#endif

#if FROM_NCOLS == 1
#define FROM_NCOLS_LOG2	0
#elif FROM_NCOLS == 2
#define FROM_NCOLS_LOG2	1
#elif FROM_NCOLS == 4
#define FROM_NCOLS_LOG2	2
#elif FROM_NCOLS == 8
#define FROM_NCOLS_LOG2	3
#else
#error Bad value for FROM_NCOLS
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
  ((fromdev_t)(x))
#elif FROM_GROUP == 2
#define fromgroup(x) \
 ({fromgroup_t __x = (fromdev_t)(x); \
   __x |= __x << (FROM_WIDTH * 8); \
 })
#elif FROM_GROUP == 4
#define fromgroup(x) \
 ({fromgroup_t __x = (fromdev_t)(x); \
   __x |= __x << (FROM_WIDTH * 8); \
   __x |= __x << (FROM_WIDTH * 8 * 2); \
 })
#endif
	
#ifndef FROM_SECGROUP
#define FROM_SECGROUP	1
#endif

#if FROM_GROUP == 1
#define foreach_word(word) \
    word = 0;
#else
#define foreach_word(word) \
    for (word = 0; word < FROM_GROUP; word++)
#endif

#if FROM_GROUP == FROM_NCOLS
#define foreach_group(col) \
    col = 0;
#else
#define foreach_group(col) \
    for (col = 0; col < FROM_NCOLS; col += FROM_GROUP)
#endif

#if FROM_SECGROUP == 1
#define foreach_sec(fcp, soffs)	\
    soffs = 0;
#else
#define foreach_sec(fcp, soffs)	\
    for (soffs = 0; soffs < FROM_SECSIZE(fcp) * FROM_NCOLS; \
	 soffs += FROM_SECSIZE(fcp) * FROM_NCOLS / FROM_SECGROUP)
#endif

/* Internal handle for device */
struct fromcookie {
    struct flashcookie		common;
    from_pt			rbase;		/* read virt address */
    from_pt			pbase;		/* programming virt address */
    paddr_t			mapbase;	/* mappable phys address */
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
    unsigned int		dlbanksz[2];	/* AM29DL bank1 size */
    unsigned int		size;		/* total region size */
    char 			*protect;
};

typedef const struct fromcookie *fromcookie_t;

#ifdef FLASHROM_INLINE
static int _flashrom_probe (struct fromcookie *, unsigned int);
#else
extern int _flashrom_probe (struct fromcookie *, unsigned int);
#endif

#endif /* __FLASHROM_H__ */
