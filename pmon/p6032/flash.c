/*
 * p6032/flash.c: PMON access to board dependent flash routines
 *
 * Copyright (c) 2000-2001, Algorithmics Ltd.  All rights reserved.
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

#include <sys/types.h>
#include <mips/cpu.h>
#include <stdio.h>
#include <mips.h>
#include <pmon.h>
#include <sde-compat.h>
#include <sbd.h>
#include <flashdev.h>

static flashcookie_t cookie;
static struct flashinfo info;

void
sbd_flashinfo (void **buf, int *size)
{
    extern char *heaptop;
    if (!cookie) {
	cookie = flash_open (0);
	if (cookie)
	    flash_info (cookie, 0, &info);
    }
    if (cookie) {
	*buf = (void *)heaptop;
	*size = info.size;
    }
    else {
	*buf = 0;
	*size = 0;
    }
}


/*
 * This is the public function is used to program the FLASH
 */
int
sbd_flashprogram (unsigned char *data, unsigned int size, unsigned int offset, 
		  int bigend)
{
    extern char _ftext[], _etext[], _edata[], _fdata[];
    int reboot = 0;
    int c, res;

    if (size + offset > info.size) {
	printf ("Too large for FLASH (offset 0x%x + size 0x%x > max 0x%x)\n", 
		offset, size, info.size);
	return 1;
    }

    /* check to see if reboot may be required (XXX dodgy test) */
    if (KVA_TO_PA (_ftext) >= BOOTPROM_BASE
	&& offset < (((_etext - _ftext) + 15) & ~15) + (_edata - _fdata))
	reboot = 1;

    printf ("Programming FLASH 0x%x-0x%x (", offset, offset+size);
    printf ("writing %s endian", bigend ? "big" : "little");
    if (reboot)
	printf (", then rebooting");
    printf (")\nAre you sure? (y/n) ");
    fflush (stdout);

    if ((c = getchar ()) != 'Y' && c != 'y') {
	printf ("\nAbandoned\n");
	return (0);
    }

    printf ("\nStarting..."); fflush (stdout);
    res = flash_programbytes (cookie, offset, data, size, 
			      FLASHDEV_PROG_MERGE 
			      | (bigend ? FLASHDEV_PROG_CODE_EB : 
				  FLASHDEV_PROG_CODE_EL)
			      | (reboot ? FLASHDEV_PROG_REBOOT : 0));

    switch (res) {
    case FLASHDEV_OK:
	printf ("completed ok\n");
	break;
    case FLASHDEV_FAIL:
	printf ("parameter error\n");
	break;
    case FLASHDEV_PROTECTED:
	printf ("can't program protected sector\n");
	break;
    case FLASHDEV_PARTIAL:
	printf ("can't erase partial sector\n");
	break;
    case FLASHDEV_NOMEM:
	printf ("no programming buffer space\n");
	break;
    case FLASHDEV_FATAL:
	printf ("fatal programming error\n");
	break;
    default:
	printf ("unknown error code %d\n", res);
	break;
    }

    return 0;
}
