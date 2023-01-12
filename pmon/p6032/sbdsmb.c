/*
 * p6032/sbdsmb.c: low level reset code for P-6032
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
#include <sys/types.h>
#include <mips/cpu.h>

#include "sbd.h"
#include "smb.h"

#include "i82371eb.h"

#define smbinb(r) inb(I82371_SMB_SMB/**/r)
#define smboutb(r,v) out(I82371_SMB_SMB/**/r,v)

static int
smb_ready(void)
{
    volatile unsigned char *smb = PA_TO_KVA1(ISAPORT_BASE(SMB_PORT));

    /* host busy ? */
    if (smbinb(HSTSTS) & I82371_SMB_HOST_BUSY)
	return 0;
    /* slave busy ? (shouldn't happen) */
    if (smbinb(SLVSTS) & I82371_SMB_SLV_BSY)
	return 0;

    /* reset interrupts */
    smboutb(HSTSTS, I82371_SMB_FAILED | I82371_SMB_BUS_ERR |
	    I82371_SMB_DEV_ERR | I82371_SMB_INTER);

    return 1;
}

static void
smb_start(unsigned int protocol)
{
    unsigned int cnt;
    /* clears the index pointer */
    cnt = smbinb(HSTCNT);
    /* set protocol and start */
    smboutb(HSTCNT, (cnt & 0xa0) | protocol | I82371_SMB_START);
}

#define SMB_ALLINTS (I82371_SMB_FAILED|I82371_SMB_BUS_ERR|I82371_SMB_DEV_ERR|I82371_SMB_INTER)

static int
smb_wait (void)
{
    unsigned int status;

    /*
     * There is latency on the HOST_BUSY bit, so monitor the interrupt
     * lines instead
     */

    do {
	status = smbinb(HSTSTS);
    } while ((status & SMB_ALLINTS) == 0); 

    smboutb(HSTSTS, status & SMB_ALLINTS);

    if (status & I82371_SMB_INTER)
	return 1;

    return 0;

}


int
_sbd_smbread(int addr, int protocol, int cmd, void *buf, int *len)
{
    unsigned char *bp = buf;
    int i;
    if (!smb_ready ())
	return 0;

    smboutb(HSTADD, addr | 1);	/* host address */
    smboutb(HSTCMD, cmd);	/* command byte */

    smb_start (protocol);
    if (smb_wait () == 0)
	return 0;

    switch (protocol) {
    case I82371_SMB_QRW:
	break;
    case I82371_SMB_BRW:
    case I82371_SMB_BDRW:
	*len = 1;
	*bp = smbinb(HSTDAT0);
	break;
    case I82371_SMB_WDRW:
	*len = 2;
	/* FIXME: endianess? */
	*bp++ = smbinb(HSTDAT0);
	*bp = smbinb(HSTDAT1);
	break;
    case I82371_SMB_BKRW:
	*len = smbinb(HSTDAT0);
	for (i = *len; i; i--)
	    *bp++ = smbinb(BLKDAT);
	break;
    }
    return 1;
}

int
_sbd_smbwrite(int addr, int protocol, int cmd, const void *buf, int len)
{
    const unsigned char *bp = buf;
    int i;
    if (!smb_ready ())
	return 0;

    smboutb(HSTADD, addr & ~1);	/* host address */
    smboutb(HSTCMD, cmd);		/* command byte */

    switch (protocol) {
    case I82371_SMB_QRW:
	break;
    case I82371_SMB_BRW:
    case I82371_SMB_BDRW:
	smboutb(HSTDAT0, *bp);
	break;
    case I82371_SMB_WDRW:
	/* FIXME: endianess ? */
	smboutb(HSTDAT0, *bp++);
	smboutb(HSTDAT1, *bp);
	break;
    case I82371_SMB_BKRW:
	smboutb(HSTDAT0, len);
	for (i = len; i; i--)
	    smboutb(BLKDAT, *bp++);
	break;
    }

    smb_start (protocol);
    return smb_wait ();
}
