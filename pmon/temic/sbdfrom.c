/*
 * TEMIC/sbdfrom.c: Flash ROM support for Algor/Temic module
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

#include "sbd.h"
#include "sbdfrom.h"
#include "flashdev.h"
#include "flashrom.h"

/* writable cookie */
static struct fromcookie cookie;


flashcookie_t
_sbd_flashopen (paddr_t devaddr)
{
    struct fromcookie *fcp = &cookie;

    if (!(devaddr == 0
	  || (devaddr >= FLASH_BASE && devaddr < FLASH_BASE + FLASH_SIZE)))
	return FLASH_UNKNOWN;

    if (fcp->size == 0) {
	/* programming base address may be different from read address */
	fcp->pbase = PA_TO_KVA1 (FLASH_BASE);
#if 0
	if (IS_KVA0 (fcp) 
	    && memcmp (PA_TO_KVA0(0x1fc0000), PA_TO_KVA1(0x1fc0000), 32) == 0)
	    /* cached accesses to PROM/FLASH work */
	    fcp->rbase = PA_TO_KVA0 (FLASH_BASE);
	else
#endif
	    /* cached accesses to PROM/FLASH fail */
	    fcp->rbase = PA_TO_KVA1 (FLASH_BASE);
	
	/* probe for first bank */
	fcp->nbanks = 1;
	if (_flashrom_probe (fcp, 0) < 0)
	    return FLASH_MISSING;

#if 0
	/* probe for second bank */
	if (_flashrom_probe (fcp, fcp->devsize << FROM_NCOLS_LOG2) >= 0)
	    fcp->nbanks = 2;
#endif

	/* compute total size */
	fcp->size= (fcp->devsize << FROM_NCOLS_LOG2) * fcp->nbanks;

#if 1
	/* h/w bug workaround */
	if (fcp->size > 0x400000)
	    fcp->size = 0x400000;
#endif

    }

    /* check we still fit */
    if (devaddr >= FLASH_BASE + fcp->size)
	return FLASH_MISSING;

    return &fcp->common;
}
