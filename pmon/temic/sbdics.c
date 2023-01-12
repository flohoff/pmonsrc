/* 
 * TEMIC/sbdics.c: ICS clock synthesiser support for Algor/Temic module
 * Copyright (c) 1999 Algorithmics Ltd.
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
#include "vrc5074.h"
#include "ics9148.h"

static volatile struct vrc5074 * const n4 = PA_TO_KVA1(VRC5074_BASE);

#define DCSFN_MASK(ch)	(N4_DCSFN_DCSFN2_MASK << ((ch-2) * 4))
#define DCSFN_GPI(ch)	(N4_DCSFN_DCSFN2_GPI << ((ch-2) * 4))
#define DCSFN_GPO(ch)	(N4_DCSFN_DCSFN2_GPO << ((ch-2) * 4))
#define DCSIO_IN(ch)	(N4_DCSIO_DCSL2IN << (ch-2))
#define DCSIO_OUT(ch)	(N4_DCSIO_DCSL2OUT << (ch-2))

#define gpio_out(ch)	n4->n4_dcsfn = ((n4->n4_dcsfn & ~DCSFN_MASK(ch)) \
					| DCSFN_GPO(ch))

#define gpio_in(ch)	n4->n4_dcsfn = ((n4->n4_dcsfn & ~DCSFN_MASK(ch)) \
					| DCSFN_GPI(ch))

#define gpio_get(ch)	(n4->n4_dcsio & DCSIO_IN(ch))

/* XXX Vrc5074 bug: it can't drive high voltages out of the DCS pins when
   they are set as gpio bits.  The workaround is to only drive 0, and to 
   switch the pin to input mode (tristate) for 1, and let an external 
   pull-up drive it high. */
#define gpio_set(ch)	gpio_in(ch)
#define gpio_clr(ch)	gpio_out(ch)

/*
 * This is the code to read/write the I2C interface on the ICS clock synth
 */
#define READ		0x01
#define WRITE		0x00

/*
 * Max chip frequency = 100 kHz, so we require at least 5 usec
 * delay between signal changes.
 */
#define WAIT() mips_cycle(CACHEUS(6))

#if __OPTIMIZE__ >= 2
#define INLINE	inline
#else
#define INLINE
#endif

/* set SCL high */
static INLINE void SCL_HI(void) {WAIT(); gpio_set(DCSIO_SCL);}
	

/* set SCL low */
static INLINE void SCL_LO(void) {WAIT(); gpio_clr(DCSIO_SCL);}
	

/* set SDA high */
static INLINE void SDA_HI(void) {WAIT(); gpio_set(DCSIO_SDA);}
	

/* set SDA low */
static INLINE void SDA_LO(void) {WAIT(); gpio_clr(DCSIO_SDA);}


/* disable SDA output driver (tristate) */
static INLINE void SDI(void) {gpio_in(DCSIO_SDA);}

	
/* enable SDA output driver (not tristate) */
static INLINE void SDO(void) {WAIT(); gpio_out(DCSIO_SDA);}


/* read SDA */
static INLINE unsigned int SDRD (void) {
    WAIT();
    return (gpio_get(DCSIO_SDA) != 0);
}
	

/* send a START: SDA high->low when SCL high */
static INLINE void START(void) {
    SDO();
    SDA_HI();
    SCL_HI();
    SDA_LO();
    SCL_LO();
}


/* send a STOP: SDA low->high when SCL high */
static INLINE void STOP(void)
{
    SDO();
    SDA_LO();
    SCL_HI();
    SDA_HI();
    SCL_LO();
    SDI();
}


/* receive an ACK: a single 0 */
static INLINE int GETACK(void)
{
    int v;
    SCL_HI();
    v = SDRD();
    SCL_LO();
    return v ^ 1;
}


/* send an ACK: a single 0 bit */
static INLINE void SENDACK(void)
{
    SDO();
    SDA_LO();
    SCL_HI();
    SCL_LO();
    SDA_HI();
    SDI();
}
	

#if 0
/* send a NACK: a single 1 bit */
static INLINE void SENDNACK(void)
{
    SDO();
    SDA_HI();
    SCL_HI();
    SCL_LO();
    SDI();
}
#endif

/* send 8 bit word (note: check for ACK externally) */
static void
i2csend8 (int v)
{
    int i;
    SDO();
    for (i = 0; i < 8; i++) {
	if (v & 0x80)
	    SDA_HI();
	else
	    SDA_LO();
	SCL_HI();
	SCL_LO();
	v <<= 1;
    }
    SDA_HI();
    SDI();
}


/* receive 8 bit word into v0 (note: send ACK/NACK externally) */
static int
i2cget8 (void)
{
    int v;
    int i;
    v = 0;
    for (i = 0; i < 8; i++) {
	SCL_HI();
	v = (v << 1) | SDRD();
	SCL_LO();
    }
    return (v);
}

/* send first command byte: device address & r/w bit */
static int
ADDR(int rw)
{
    START();
    i2csend8 (0xd2 | rw);
    return GETACK();
}

static void
_sbd_icsreset (void)
{
    gpio_in (DCSIO_ICS);	/* tristate ICS select (pulled high) */
    SCL_LO();			/* initialise SCL to 0 */
    SDA_LO();			/* initialise SDA to 0 */
#if 0
    {
	int i;
	for (i = 0; i < 9; i++)
	    STOP();
    }
#endif
}


int 
_sbd_icsread (void *buf, int size)
{	
    unsigned char *bp;
    int nb, i;

    _sbd_icsreset ();

    /* send read address */
    if (!ADDR(READ))
	return -1;

    /* get byte count */
    nb = i2cget8 ();
    SENDACK();
    if (nb > size)
	nb = size;

    /* read data bytes */
    for (bp = buf, i = 0; i < nb; i++) {
	*bp++ = i2cget8 ();
	SENDACK();
    }

    STOP();
    return nb;
}


int 
_sbd_icswrite (const void *buf, int nb)
{	
    const unsigned char *bp;
    int i;

    _sbd_icsreset ();

    /* send write address */
    if (!ADDR(WRITE))
	return -1;
    
    /* send dummy command */
    i2csend8 (0);
    if (!GETACK())
	return -1;

    /* send dummy byte count */
    i2csend8 (nb);
    if (!GETACK())
	return -1;

    /* send data bytes */
    for (bp = buf, i = 0; i < nb; i++) {
	i2csend8 (*bp++);
	if (!GETACK())
	    break;
    }

    STOP();
    return i;
}
