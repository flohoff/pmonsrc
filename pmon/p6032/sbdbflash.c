/*
 * p6032/sbdbflash.c: Boot flash ROM support for P-6032
 *
 * Copyright (c) 2000 Algorithmics Ltd - all rights reserved.
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

#ifdef IN_PMON
#include <sys/types.h>
#include <mips/cpu.h>
#include "pmon.h"
#include <assert.h>
#else
#include <sys/types.h>
#include <mips/cpu.h>
#include <stdlib.h>
#include "kit_impl.h"
#endif

#include "flashdev.h"
#include "flashrom.h"
#include "sbd.h"

extern int _flash8_probe (struct fromcookie *, unsigned int);
extern int _flash16_probe (struct fromcookie *, unsigned int);

/* writable cookie */
static struct fromcookie flash_cookie;
static struct fromcookie boot_cookie;

/* board-specific code */
static int fromgetparts (flashcookie_t, struct flashparts *);
static int fromsetparts (flashcookie_t, const struct flashparts *);

static const struct flashboard fromboard = {
    fromgetparts,
    fromsetparts
};


static char * const _bonito = PA_TO_KVA1(BONITO_BASE);

flashcookie_t
_sbd_bflashopen (paddr_t devaddr)
{
    struct fromcookie *fcp;
    paddr_t base;

    if (devaddr == 0
	 || (devaddr >= SOCKET_BASE && devaddr < SOCKET_BASE + SOCKET_SIZE)) {
	fcp = &flash_cookie;
	base = SOCKET_BASE;
    }
    else if (devaddr >= BOOT_BASE && devaddr < BOOT_BASE + BOOT_SIZE
	     && (BONITO_BONPONCFG & BONITO_BONPONCFG_ROMBOOT) == BONITO_BONPONCFG_ROMBOOT_SOCKET) {
	/* XXX boot window maps to start of socket */
	fcp = &boot_cookie;
	base = BOOTPROM_BASE;
    }
    else
	return FLASH_UNKNOWN;

    if (fcp->size == 0) {
	/* always program via flash address */
	fcp->pbase = PA_TO_KVA1 (SOCKET_BASE);

	/* read from there too, but cached */
	if (IS_KVA0 (fcp))
	    fcp->rbase = PA_TO_KVA0 (SOCKET_BASE);
	else
	    fcp->rbase = PA_TO_KVA1 (SOCKET_BASE);
	
	fcp->nbanks = 1;
	
	/* probe for device */
	if (BONITO_BONPONCFG & BONITO_BONPONCFG_ROMCS0WIDTH) {
	    if (_flash16_probe (fcp, 0) < 0)
		return FLASH_MISSING;
	}
	else {
	    if (_flash8_probe (fcp, 0) < 0)
		return FLASH_MISSING;
	}
	
	/* compute total size */
	fcp->size = fcp->devsize;	/* one device */

	/* device may also be mappable */
	fcp->mapbase = base;
    }

    /* check we still fit */
    if ((devaddr & 0xfffff) >= fcp->size)
	return FLASH_MISSING;

    /* plug in board-specific functions */
    fcp->common.board = &fromboard;

    return &fcp->common;
}


/*
 * Return flash partition table 
 */
static int
fromgetparts (flashcookie_t cookie, struct flashparts *parts)
{
    struct fromcookie *fcp = (struct fromcookie *)cookie;

    memset (parts, 0, sizeof (*parts));
    _flashdev_setpart (parts, FLASHPART_RAW, FLASHPART_RAW, 0, fcp->size);
    _flashdev_setpart (parts, FLASHPART_BOOT, FLASHPART_BOOT, 0, 0x68000);
    _flashdev_setpart (parts, FLASHPART_POST, FLASHPART_POST, 
		       0x68000, 0x18000);
#if _SBD_FLASHENV==0
    {
	/* find at least 512 bytes at top of flash for environment */
	struct flashinfo info;
	unsigned int eoffs, esize;

	flash_info (cookie, 0, &info);
	eoffs = info.size; esize = 0; 
	while (esize < 512) {
	    flash_info (cookie, eoffs - 1, &info);
	    eoffs = info.soffs;
	    if (info.sprot) /* must be writable */
		esize = 0;
	    else
		esize += info.ssize;
	}
	_flashdev_setpart (parts, FLASHPART_ENV, FLASHPART_ENV, 
			   eoffs, esize);
	if (eoffs < 0x80000)
	    /* reduce POST size for environment */
	    parts->part[FLASHPART_POST].size = eoffs
		- parts->part[FLASHPART_POST].offs;
	else
	    /* spare partition */
	    _flashdev_setpart (parts, FLASHPART_FFS, FLASHPART_UNDEF, 
			       0x80000, eoffs - 0x80000);
    }
#else
    if (fcp->size > 0x80000)
	/* spare partition */
	_flashdev_setpart (parts, FLASHPART_FFS, FLASHPART_UNDEF, 
			   0x80000, fcp->size - 0x80000);

#endif
    return FLASHDEV_OK;
}


static int
fromsetparts (flashcookie_t cookie, const struct flashparts *parts)
{
    /* not supported yet, sorry */
    return FLASHDEV_FAIL;
}
