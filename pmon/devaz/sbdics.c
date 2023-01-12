/* 
 * devaz/sbdics.c: ICS clock synthesiser support for Deva-0
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

static char * const _bonito = PA_TO_KVA1(BONITO_BASE);
	
#define gpio_get()	BONITO_GPIODATA
#define gpio_bis(x)	BONITO_GPIODATA |= (x)
#define gpio_bic(x)	BONITO_GPIODATA &= ~(x)
#define gpioie_bis(x)	BONITO_GPIOIE |= (x)
#define gpioie_bic(x)	BONITO_GPIOIE &= ~(x)

/*
 * This is the code to read/write the I2C interface on the ICS clock synth
 */
#define READ		0x01
#define WRITE		0x00

/*
 * Max chip frequency = 100 kHz, so we require at least 5 usec
 * delay between signal changes.
 */
#define WAIT() usdelay(6)


#if __OPTIMIZE__ >= 2
#define INLINE	inline
#else
#define INLINE
#endif

/* set SCL high */
static INLINE void SCL_HI(void) {WAIT(); gpio_bis(PIO_ICS_SCL);}
	

/* set SCL low */
static INLINE void SCL_LO(void) {WAIT(); gpio_bic(PIO_ICS_SCL);}
	

/* set SDA high */
static INLINE void SDA_HI(void) {WAIT(); gpio_bis(PIO_ICS_SDAW);}
	

/* set SDA low */
static INLINE void SDA_LO(void) {WAIT(); gpio_bic(PIO_ICS_SDAW);}


/* disable SDA output driver (tristate) */
static INLINE void SDI(void) {
    WAIT(); \
    gpioie_bis(PIO_ICS_SDAW);
}

	
/* enable SDA output driver (not tristate) */
static INLINE void SDO(void) {
    WAIT();
    gpioie_bic(PIO_ICS_SDAW);
}


/* read SDA */
static INLINE unsigned int SDRD (void) {
    WAIT();
    return (gpio_get() & PIO_ICS_SDAR) != 0;
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
