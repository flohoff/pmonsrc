/* 
 * GAL9/sbdflashenv.c: board-specific environment in Flash
 * Copyright (c) 1997  Algorithmics Ltd
 */

#ifdef IN_PMON
#include "pmon.h"
#include "sde-compat.h"
#else
#include <stddef.h>
#include <mips/cpu.h>
#include <kit_impl.h>
#endif
#include <flashdev.h>
#include <sbd.h>

flashcookie_t
_sbd_flashenv_open (unsigned int *env_offs, unsigned int *env_maxsize)
{
    flashcookie_t cookie;
    struct flashinfo info;
    int offs;
    unsigned int eoffs, esize;

    /* XXX board specific address - wired-down flash prom */
    cookie = flash_open (FLASH_BASE);
    if (!cookie)
	cookie = flash_open (BOOTPROM_BASE);
    if (!cookie)
	return (flashcookie_t) 0;

    /* find flash size */
    if (flash_info (cookie, 0, &info) != FLASHDEV_OK)
	return (flashcookie_t) 0;

    /* find the smallest writable sector from the top of the flash */
    offs = info.size;
    eoffs = esize = 0;
    while (offs > 0) {
	flash_info (cookie, offs - 1, &info);
	offs -= info.ssize;
	if (info.sprot)
	    continue;
	if (eoffs == 0 || info.ssize < esize) {
	    eoffs = info.soffs;
	    esize = info.ssize;
	}
	/* stop when we reach the non-boot sectors */
	if (info.ssize >= info.maxssize)
	    break;
    }
    if (esize == 0)
	return (flashcookie_t)0;

    *env_offs = eoffs;
    *env_maxsize = esize;

    return cookie;
}
