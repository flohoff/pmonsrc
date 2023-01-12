/*
 * p6032/ns16550.c: PMON serial port driver for P6032/P6064
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

#include <mips.h>
#include <termio.h>
#include <pmon.h>
#include <sbd.h>
#include <ns16550.h>


static int nsfifo;

static int
nsinit (volatile ns16550dev *dp, int chan)
{
    unsigned int x;


    /* force access to id reg */
    dp->cfcr = 0;
    dp->iir = 0;
    if ((dp->iir & 0x38) != 0) {
	sbdmessage (0, "EIIR");
	return 1;
    }

    /* in case it really is a 16550, enable the fifos */
    dp->fifo = FIFO_ENABLE;
    sbddelay(100);
    dp->fifo = FIFO_ENABLE | FIFO_RCV_RST | FIFO_XMT_RST | FIFO_TRIGGER_1;
    sbddelay(100);

    if ((dp->iir & IIR_FIFO_MASK) == IIR_FIFO_MASK)
	nsfifo = 1;
    else {
	sbdmessage (0, "NFFO");
	dp->fifo = 0;
    }

    return 0;
}


static int
nsprogram (volatile ns16550dev *dp, int chan, int baudrate)
{
    unsigned short brtc;
    int timeout;

    /* wait for Tx fifo to completely drain */
    timeout = 10000;
    while (!(dp->lsr & LSR_TSRE))
	if (--timeout == 0)
	    break;

    baudrate = getbaudval (baudrate);
    if (baudrate == 0)
      return 1;

    brtc = BRTC (baudrate);

    dp->cfcr = CFCR_DLAB;
    dp->data = brtc & 0xff;
    dp->ier = brtc >> 8;
    dp->cfcr = CFCR_8BITS;
    dp->mcr = MCR_DTR | MCR_RTS | MCR_IENABLE;
    dp->ier = 0;
    return 0;
}


ns16550 (int op, char *dat, int chan, int data)
{
    volatile ns16550dev *dp = (ns16550dev *) dat;

    if (chan > 1)
	return 1;

    switch (op) {
    case OP_INIT:
	return nsinit (dp, chan);

    case OP_XBAUD:
    case OP_BAUD:
	return nsprogram (dp, chan, data);

    case OP_TXRDY:
	return (dp->lsr & LSR_TXRDY);

    case OP_TX:
	dp->data = data; wbflush ();
	break;

    case OP_RXRDY:	
	return (dp->lsr & LSR_RXRDY);

    case OP_RX:
	return dp->data & 0xff;

    case OP_RXSTOP:
	if (data)
	    dp->mcr &= ~MCR_RTS;	    /* disable Rx */
	else
	    dp->mcr |= MCR_RTS;	    	    /* enable Rx */
	break;
    }
    return 0;
}
