/* 
 * p6032/sbdics.c: ICS clock synthesiser support for P-6032
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
#include <pmon.h>
#include <sde-compat.h>
#else
#include <stddef.h>
#include <mips/cpu.h>
#include "kit_impl.h"
#endif
#include "sbd.h"

#include "smb.h"
#include "ics9148.h"

int 
_sbd_icsread (void *buf, int size)
{	
    int l;
    return _sbd_smbread (ICS_I2C_BASE_ADDRESS, SMB_BKRW, 0, buf, &l);
}


int 
_sbd_icswrite (const void *buf, int nb)
{	
    return _sbd_smbwrite (ICS_I2C_BASE_ADDRESS, SMB_BKRW, 0, buf, nb);
}

