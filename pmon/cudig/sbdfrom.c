/*
 * CUDIG/sbdfrom.c: Flash ROM support for Telegate CUDIG
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
#include <kit_impl.h>
#endif

#include "sbd.h"
#include "sbdfrom.h"
#include "flashdev.h"
#include "flashrom.h"

/* one flash cookie for each possible flash rom in system */
static struct fromcookie cookies[] = {
    {{FLASH_BASE, 	FLASH_SIZE}},
    {{0}}
};

flashcookie_t
_sbd_flashopen (paddr_t devaddr)
{
    struct fromcookie *fcp;

    if (devaddr < sizeof(cookies)/sizeof(cookies[0])) {
	/* devaddr is an index */
	fcp = &cookies[devaddr];
    }
    else {
	/* search for device addressed by devaddr */
	for (fcp = cookies; fcp->common.size != 0; fcp++)
	    if (fcp->common.base <= devaddr 
		&& devaddr < fcp->common.base + fcp->common.size)
		break;
    }

    if (fcp->common.size == 0)
	/* unknown device number/address */
	return (flashcookie_t)-1;

    if (fcp->common.dev)
	/* already initialised */
	return &fcp->common;

    /* programming base address may be different from read address */
    fcp->pbase = PA_TO_KVA1 (fcp->common.base);
    if (IS_KVA0 (fcp))
	fcp->rbase = PA_TO_KVA0 (fcp->common.base);
    else
	fcp->rbase = PA_TO_KVA1 (fcp->common.base);

    /* flash device geometry (fixed on this board) */
    fcp->nbanks		= 1;			/* 1 sequential bank */

    /* XXX scan for actual number of columns and banks, if variable */
    if (_flashrom_find (fcp, 0) < 0)
	return NULL;

    /* compute total size */
    fcp->common.size= (fcp->devsize << FROM_NCOLS_LOG2) * fcp->nbanks;

    /* check we still fit */
    if (devaddr > fcp->common.base + fcp->common.size)
	return NULL;

    return &fcp->common;
}
