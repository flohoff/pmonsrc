/*
 * p6032/sbdflash16.c: 16 bit flash ROM support for P-6032
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
#else
#include <sys/types.h>
#include <mips/cpu.h>
#include <stdlib.h>
#include "kit_impl.h"
#endif

#include "flash16.h"
#include "flashdev.h"
#define FLASHROM_INLINE
#include "flashrom.h"
#include "sbd.h"

int
_flash16_probe (struct fromcookie *fcp, unsigned int o)
{
    return _flashrom_probe (fcp, o);
}

/* include code inline since sbdflash8.c will need a different copy */
#include "flashrom.c"
