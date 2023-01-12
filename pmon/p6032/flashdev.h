/*
 * share/flashdev.h: generic FLASH support definitions
 *
 * Copyright (c) 1996-1999, Algorithmics Ltd.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the "Free MIPS" License Agreement, a copy of 
 * which is available at:
 *
 *  http://www.algor.co.uk/ftp/pub/doc/freemips-license.txt
 *
 * You may not, however, modify or remove any part of this copyright 
 * message if this program is redistributed or reused in whole or in
 * part.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * "Free MIPS" License for more details.  
 */

#ifndef _flashdev_H_
#define _flashdev_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A cookie passed to the device routines that may be used
 * to control system/device specific information (eg write protection)
 * Note: private to flash routines, caller should treat as opaque.
 */
struct flashcookie {
    const struct flashdev *	dev;		/* dev access functions */
    const struct flashboard *	board;		/* board access functions */
};

/* this is what the caller should use */
typedef const struct flashcookie *flashcookie_t;

/* this function returns the cookie for the flash at the specified 
   physical address, or index (0..n) */
extern flashcookie_t	_sbd_flashopen (paddr_t);

/* call this for POSIX /dev/flash device interface */
extern void		_flashdev_load (void);

/* missing: valid address or index, but no device found */
#define FLASH_MISSING	(flashcookie_t)0

/* unknown: unknown address or device index */
#define FLASH_UNKNOWN	(flashcookie_t)-1

/*
 * This buffer is used when reprogramming a partial sector.
 * It must be large enough to hold the largest possible sector size.
 */
extern unsigned char	_sbd_flash_secbuf[];

/*
 * Information structure returned by the flashinfo() function
 */
struct flashinfo {
    char 		name[32];	/* dev name */
    paddr_t		base;		/* dev base (phys address) */
    unsigned int	size;		/* dev size */
    paddr_t		mapbase; 	/* memory mapped base (phys addr) */
    unsigned char	unit;		/* unit byte size (1,2,4,8 or 16) */
    unsigned int	maxssize; 	/* maximum sector size */
    unsigned int	soffs;		/* base offset of specified sector */
    unsigned int	ssize;		/* size of specified sector */
    int			sprot;		/* specified sector is protected */
};


struct flashpart {
    int			type;		/* partition type */
    unsigned int	offs;		/* base of partition */
    unsigned int	size;		/* size of partition */
};

/* max number of partitions */
#define FLASHNPART	8
struct flashparts {
    struct flashpart	part[FLASHNPART];
};


/* partition types */
#define FLASHPART_RAW		0	/* raw (whole) device */
#define FLASHPART_BOOT		1	/* boot partition */
#define FLASHPART_POST		2	/* post partition (self test) */
#define FLASHPART_ENV		3	/* non-volatile environment */
#define FLASHPART_FFS		4	/* flash file system partition */
#define FLASHPART_UNDEF		255	/* undefined partition */

/* magic number preceding partition table if stored in flash */
#define FLASHPART_MAGIC		0x77049081

/* define a partition structure dynamically */
#define _flashdev_setpart(parts, idx, t, o, s) \
  do { \
    struct flashpart *_p = &(parts)->part[idx]; \
    _p->type = t; _p->offs = o; _p->size = s; \
  } while (0)
    
/*
 * Vectors to device specific routines
 */
struct flashdev {
    int	(*devinfo) (flashcookie_t, unsigned int, struct flashinfo *);
    int	(*deverasedevice) (flashcookie_t);
    int	(*deverasesector) (flashcookie_t, unsigned int);
    int	(*devprogrambytes) (flashcookie_t, unsigned int, 
			    const void *, unsigned int, int);
    int	(*devreadbytes) (flashcookie_t, unsigned int, 
			 void *, unsigned int, unsigned int);
};


/*
 * Vectors to board specific routines
 */
struct flashboard {
    int (*getparts) (flashcookie_t, struct flashparts *);
    int (*setparts) (flashcookie_t, const struct flashparts *);
};

/* cover macros */
#define flash_open(paddr) \
    _sbd_flashopen (paddr)

#define flash_info(cookie, offs, infop) \
    (*(cookie)->dev->devinfo) (cookie, offs, infop)

#define flash_erasedevice(cookie) \
    (*(cookie)->dev->deverasedevice) (cookie)

#define flash_erasesector(cookie, soffs) \
    (*(cookie)->dev->deverasesector) (cookie, soffs)

#define flash_programbytes(cookie, offs, buf, len, flags) \
    (*(cookie)->dev->devprogrambytes) (cookie, offs, buf, len, flags)

#define flash_flush(cookie) \
    (*(cookie)->dev->devprogrambytes) (cookie, 0, 0, 0, 0)

#define flash_readbytes(cookie, offs, buf, len, flags) \
    (*(cookie)->dev->devreadbytes) (cookie, offs, buf, len, flags)

#define flash_parts(cookie, parts) \
    (*(cookie)->board->getparts) (cookie, parts)

#define flash_setparts(cookie, parts) \
    (*(cookie)->board->setparts) (cookie, parts)

/* status returned by above functions */
#define FLASHDEV_OK		 0	/* success */
#define FLASHDEV_FAIL		-1	/* parameter error */
#define FLASHDEV_PROTECTED	-2	/* can't program protected sector */
#define FLASHDEV_PARTIAL	-3	/* can't erase partial sector */
#define FLASHDEV_NOMEM		-4	/* no memory for buffer */
#define FLASHDEV_FATAL		-5	/* fatal programming error */

/* flags parameter to flash_programbytes() */
#define FLASHDEV_PROG_REBOOT	0x01	/* reboot after programming */
#define FLASHDEV_PROG_NOCOPY	0x02	/* don't copy code to dram */
#define FLASHDEV_PROG_MERGE	0x04	/* merge partially written sectors */
#define FLASHDEV_PROG_CODE_EB	0x08	/* write big-endian code */
#define FLASHDEV_PROG_CODE_EL	0x00	/* write little-endian code */
#if #endian(big)
#define FLASHDEV_PROG_CODE	FLASHDEV_PROG_CODE_EB
#else
#define FLASHDEV_PROG_CODE	FLASHDEV_PROG_CODE_EL
#endif
#define FLASHDEV_PROG_STREAM	0x10	/* write byte stream */
#define _FLASHDEV_SECTOR	0x20	/* sector access (internal flag) */
#define _FLASHDEV_CMD		0x40	/* command access (internal flag) */

/* flags parameter to flash_readbytes() */
#define FLASHDEV_READ_CODE_EB	0x08
#define FLASHDEV_READ_CODE_EL	0x00
#define FLASHDEV_READ_STREAM	0x10	/* read byte stream */
#if #endian(big)
#define FLASHDEV_READ_CODE	FLASHDEV_READ_CODE_EB
#else
#define FLASHDEV_READ_CODE	FLASHDEV_READ_CODE_EL
#endif

#define FLASHDEV_MODE		0x78

/* 
 * ioctl based interface 
 */
#ifdef _IOWR
#define	FLASHIOINFO	 _IOWR('h', 0, struct flashinfo)/* get flash info */
#define	FLASHIOGPART	 _IOR('h', 1, struct flashpart) /* get partition */
#define	FLASHIOGFLGS	 _IOR('h', 2, unsigned int)	/* get flags */
#define	FLASHIOSFLGS	 _IOW('h', 3, unsigned int)	/* set flags */
#define	FLASHIOERASEDEV	 _IO('h', 4)			/* erase device */
#define	FLASHIOERASESECT _IOW('h', 5, unsigned int)	/* erase sector */
#define	FLASHIOGPARTS	 _IOR('h', 7, struct flashparts)/* get all partns */
#define	FLASHIOSPARTS	 _IOR('h', 8, struct flashparts)/* set all partns */
#define	FLASHIOFLUSH	 _IO('h', 9) 			/* flush writes */
#endif

#ifdef __cplusplus
}
#endif

#endif /* _flashdev_H_ */
