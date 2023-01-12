/*
 * share/flashdev.h: generic FLASH support definitions
 * Copyright (c) 1996 Algorithmics Ltd.
 */

#ifndef _flashdev_H_
#define _flashdev_H_

/*
 * A cookie passed to the device routines that may be used
 * to control system/device specific information (eg write protection)
 * Note: private to flash routines, caller should treat as opaque.
 */
struct flashcookie {
    paddr_t			base;	/* device base (physical address) */
    unsigned int		size;	/* device total size */
    unsigned int		unit;	/* device unit size */
    const struct flashdev *	dev;	/* access functions */
    const void *		data;	/* device specific */
};

/* this is what the caller should use */
typedef const struct flashcookie *flashcookie_t;

/* this function returns the cookie for the flash at the specified 
   physical address */
extern flashcookie_t	_sbd_flashopen (paddr_t);

/*
 * This buffer is used when reprogramming a partial sector.
 * It must be large enough to hold the largest possible sector size.
 */
extern unsigned char	_sbd_flash_secbuf[];

/*
 * Information structure returned by the flashinfo() function
 */
struct flashinfo {
    const char *	name;		/* dev name */
    paddr_t		base;		/* dev base (phys address) */
    unsigned int	size;		/* dev size */
    paddr_t		mapbase; 	/* memory mapped base (phys addr) */
    unsigned char	unit;		/* unit byte size (1,2,4,8 or 16) */
    unsigned int	maxssize; 	/* maximum sector size */
    unsigned int	soffs;		/* base offset of specified sector */
    unsigned int	ssize;		/* size of specified sector */
    int			sprot;		/* specified sector is protected */
};


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

/* cover macros */
#define flash_open(paddr) \
    _sbd_flashopen (paddr)

#define flash_info(cookie, offs, infop) \
    (*(cookie)->dev->devinfo) (cookie, offs, infop)

#define flash_erasedevice(cookie)->dev \
    (*(cookie)->dev->deverasedevice) (cookie)

#define flash_erasesector(cookie, soffs) \
    (*(cookie)->dev->deverasesector) (cookie, soffs)

#define flash_programbytes(cookie, offs, buf, len, flags) \
    (*(cookie)->dev->devprogrambytes) (cookie, offs, buf, len, flags)

#define flash_readbytes(cookie, offs, buf, len, flags) \
    (*(cookie)->dev->devreadbytes) (cookie, offs, buf, len, flags)

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

/* flags parameter to flash_readbytes() */
#define FLASHDEV_READ_CODE_EB	0x08
#define FLASHDEV_READ_CODE_EL	0x00
#define FLASHDEV_READ_STREAM	0x10	/* read byte stream */
#if #endian(big)
#define FLASHDEV_READ_CODE	FLASHDEV_READ_CODE_EB
#else
#define FLASHDEV_READ_CODE	FLASHDEV_READ_CODE_EL
#endif

#endif /* _flashdev_H_ */
