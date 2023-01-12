/*
 * devaz/sbduflash.c: User flash ROM support for Deva-0
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

#include "uflash.h"
#include "flashdev.h"
#define FLASHROM_INLINE
#include "flashrom.h"
#include "sbd.h"

/* writable cookie */
static struct fromcookie socket_cookie;
static struct fromcookie boot_cookie;

static char * const _bonito = PA_TO_KVA1(BONITO_BASE);


flashcookie_t
_sbd_uflashopen (paddr_t devaddr)
{
    struct fromcookie *fcp;
    paddr_t base;

    if (devaddr == 0
	 || (devaddr >= FLASH_BASE && devaddr < FLASH_BASE + FLASH_SIZE)) {
	fcp = &socket_cookie;
	base = FLASH_BASE;
    }
    else if (devaddr >= BOOT_BASE && devaddr < BOOT_BASE + BOOT_SIZE
	     && (BONITO_BONPONCFG & BONITO_BONPONCFG_ROMBOOT) == BONITO_BONPONCFG_ROMBOOT_FLASH) {
	/* XXX boot window maps to start of flash */
	fcp = &boot_cookie;
	base = BOOTPROM_BASE;
    }
    else
	return FLASH_UNKNOWN;

    if (fcp->size == 0) {
	/* always program via flash address */
	fcp->pbase = PA_TO_KVA1 (FLASH_BASE);

	/* read from there too, but cached */
	if (IS_KVA0 (fcp))
	    fcp->rbase = PA_TO_KVA0 (FLASH_BASE);
	else
	    fcp->rbase = PA_TO_KVA1 (FLASH_BASE);
	
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
