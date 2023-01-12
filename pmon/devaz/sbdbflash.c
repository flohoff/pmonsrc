/*
 * devaz/sbdbflash.c: Boot flash ROM support for Deva-0
 * Copyright (c) 1999 Algorithmics Ltd.
 */

#ifdef IN_PMON
#include "pmon.h"
#include "sde-compat.h"
#include <assert.h>
#else
#include <sys/types.h>
#include <mips/cpu.h>
#include <stdlib.h>
#include "kit_impl.h"
#endif

#include "bflash.h"
#include "flashdev.h"
#define FLASHROM_INLINE
#include "flashrom.h"
#include "sbd.h"

/* writable cookie */
static struct fromcookie flash_cookie;
static struct fromcookie boot_cookie;

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
	if (_flashrom_probe (fcp, 0) < 0)
	    return FLASH_MISSING;
	
	/* compute total size */
	fcp->size = fcp->devsize * FROM_NCOLS;

	/* device may also be mappable */
	fcp->mapbase = base;
    }

    /* check we still fit */
    if ((devaddr & 0xfffff) >= fcp->size)
	return FLASH_MISSING;

    return &fcp->common;
}


/* include code inline since sbduflash.c will need a different copy */
#include "flashrom.c"
