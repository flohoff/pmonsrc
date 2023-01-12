/* $Id: pio.c,v 1.1 1996/12/10 11:58:26 nigel Exp $ */

/*
 * pio.c: routines to handle parallel i/o devices on Laserfold boards
 */


#include <mips.h>
#include <pmon.h>
#include <sbd.h>
#include <pio.h>

struct piomap {
    unsigned int	port;
    unsigned int	bits;
    unsigned int	(*get)(const struct piomap *);
    void		(*put)(const struct piomap *, unsigned int);
};


/* shared write-only register state (just below general exception vector) */
static unsigned char	* const piosoft = 
	(unsigned char *)(0xa0000080-4);
static unsigned char * const piostype = 
	(unsigned char *)(0xa0000080-5);
#define piotype		(*piostype)


/* dummy put/get functions */

static unsigned int softget (const struct piomap *map)
{
    return piosoft[map->port];
}


static unsigned int noget (const struct piomap *map)
{
    return 0;
}


static void noput (const struct piomap *map, unsigned int val)
{
}



/*
 * Intel i8255 specific i/o
 */

#include <i8255.h>

#define I8255(port) \
  (volatile unsigned short *)PHYS_TO_K1(IOGPCHAN0_BASE + (port) * 4)

/* 800ns delay between accesses */
#define i8255wait()	sbddelay(1)


static void i8255reset (void)
{
    int port;
    /* reset all ports, because changing the mode clears them */
    for (port = I8255_CTL; port >= I8255_PA; port--) {
	volatile unsigned short *i8255 = I8255(port);
	*i8255 = piosoft[port];
	i8255wait ();
    }
}


static unsigned int i8255geta (const struct piomap *map)
{
    unsigned char nctl = I8255_MODE_A_IN; 
    unsigned char val;

    if (nctl != piosoft[I8255_CTL]) {
	/* new mode */
	piosoft[I8255_CTL] = nctl;
	i8255reset();
    }

    val = *I8255(I8255_PA);
    i8255wait ();
    return val;
}


static void i8255puta (const struct piomap *map, unsigned int val)
{
    unsigned char nctl = I8255_MODE_A_OUT; 

    if (nctl != piosoft[I8255_CTL]) {
	/* new mode */
	piosoft[I8255_CTL] = nctl;
	piosoft[I8255_PA] = val;
	i8255reset();
    }
    else {
	*I8255(I8255_PA) = (piosoft[I8255_PA] = val);
	i8255wait ();
    }
}


static unsigned int i8255getbc (const struct piomap *map)
{
    unsigned char val;
    val = *I8255(map->port);
    i8255wait ();
    return val;
}


static void i8255putbc (const struct piomap *map, unsigned int val)
{
    volatile unsigned short *i8255 = I8255(map->port);
    *i8255 = (piosoft[map->port] = val);
    i8255wait ();
}


static int i8255init (void)
{
    unsigned int in, out;

    /* see if port value changes when we change mode */
    *I8255(I8255_CTL) = I8255_MODE_A_OUT; i8255wait();
    out = *I8255(I8255_PA) & 0x7f; i8255wait();
    *I8255(I8255_CTL) = I8255_MODE_A_IN; i8255wait();
    in = *I8255(I8255_PA) & 0x7f; i8255wait();
    if (in == out)
	/* no change: must be other pio */
	return 0;

    piosoft[I8255_CTL]= I8255_MODE_A_IN;
    piosoft[I8255_PA] = 0;
    piosoft[I8255_PB] = PB_LED_NREADY | PB_LED_NDATA | PB_LED_NONLINE;
    piosoft[I8255_PC] = PC_PP_NSEL2;
    i8255reset ();
    return 1;
}


static const struct piomap i8255map[] = {
/* PIO_SW */
    {I8255_PA,	0x7f,		i8255geta,	noput},
/* PIO_LCD_BUSY */
    {I8255_PA,	PA_LCD_BUSY,	i8255geta,	noput},
/* PIO_LCD_DATA */
    {I8255_PA,	0xff,		noget,		i8255puta},
/* PIO_LCD_RW */
    {I8255_PB,	PB_LCD_RW,	softget,	i8255putbc},
/* PIO_LCD_E */
    {I8255_PB,	PB_LCD_E,	softget,	i8255putbc},
/* PIO_LCD_RS */
    {I8255_PB,	PB_LCD_RS,	softget,	i8255putbc},
/* PIO_LED_NREADY */
    {I8255_PB,	PB_LED_NREADY,	softget,	i8255putbc},
/* PIO_LED_NDATA */
    {I8255_PB,	PB_LED_NDATA,	softget,	i8255putbc},
/* PIO_LED_NONLINE */
    {I8255_PB,	PB_LED_NONLINE,	softget,	i8255putbc},
/* PIO_E2_DIN */
    {I8255_PB,	PB_E2_DIN,	softget,	i8255putbc},
/* PIO_E2_SK */
    {I8255_PB,	PB_E2_SK,	softget,	i8255putbc},
/* PIO_E2_CS */
    {I8255_PB,	PB_E2_CS,	softget,	i8255putbc},
/* PIO_E2_DOUT */
    {I8255_PC,	PC_E2_DOUT,	i8255getbc,	noput},
/* PIO_TEMP_2 */
    {I8255_PC,	PC_TEMP_2,	softget,	i8255putbc},
/* PIO_TEMP_1 */
    {I8255_PC,	PC_TEMP_1,	softget,	i8255putbc},
/* PIO_TEMP_0 */
    {I8255_PB,	PB_TEMP_0,	softget,	i8255putbc},
/* PIO_PP_NSEL2 */
    {I8255_PC,	PC_PP_NSEL2,	softget,	i8255putbc},
/* PIO_PP_NSEL1 */
    {I8255_PC,	PC_PP_NSEL1,	softget,	i8255putbc},
/* PIO_ENGSEL */
    {I8255_PC,	PC_ENGSEL,	i8255getbc,	noput},
};



/*
 * No PIO on new board, just a set of registers.
 */

#include <npio.h>

#define NPIO(port) \
  (volatile unsigned short *)PHYS_TO_K1(IOGPCHAN0_BASE + (port) * 4)

static int npioinit (void)
{
    /* switch off all LEDs; disable output to LCD panel */
    piosoft[NPIO_PB] = NPB_LED_NREADY | NPB_LED_NDATA | NPB_LED_NONLINE
	| NPB_LCD_NOUTPUT;

    /* select parallel port #1; no E2ROM chip select */
    piosoft[NPIO_PC] = NPC_PP_NSEL2;

    *NPIO(NPIO_PB) = piosoft[NPIO_PB];
    *NPIO(NPIO_PC) = piosoft[NPIO_PC];

    return 1;
}


static void npioputlcde (const struct piomap *map, unsigned int val)
{
    /* LCD E signal is changing */

    if ((val & (NPB_LCD_RW | NPB_LCD_E)) == NPB_LCD_E) {
	/* start of E write pulse: enable output to LCD */
	val &= ~NPB_LCD_NOUTPUT;
    }
    else if ((val & (NPB_LCD_RW | NPB_LCD_E)) == 0) {
	/* end of E write pulse: disable output to LCD, but not until
	   we've taken away E, to guarantee data hold time. */
	*NPIO(NPIO_PB) = val;
	val |= NPB_LCD_NOUTPUT;
    }

    *NPIO(NPIO_PB) = (piosoft[NPIO_PB] = val);
}


static unsigned int npiogeta (const struct piomap *map)
{
    if (!(piosoft[NPIO_PB] & NPB_LCD_NOUTPUT)) {
	/* disable output to LCD */
	*NPIO(NPIO_PB) = (piosoft[NPIO_PB] |= NPB_LCD_NOUTPUT);
    }
    return *NPIO(NPIO_PA);
}


static void npioputa (const struct piomap *map, unsigned int val)
{
    if (piosoft[NPIO_PB] & NPB_LCD_NOUTPUT) {
	/* enable output to LCD */
	*NPIO(NPIO_PB) = (piosoft[NPIO_PB] &= ~NPB_LCD_NOUTPUT);
    }
    *NPIO(NPIO_PA) = (piosoft[NPIO_PA] = val);
}


static unsigned int npiogetbc (const struct piomap *map)
{
    return *NPIO(map->port);
}


static void npioputbc (const struct piomap *map, unsigned int val)
{
    *NPIO(map->port) = (piosoft[map->port] = val);
}


static const struct piomap npiomap[] = {
/* PIO_SW */
    {NPIO_PA,	0x7f,		npiogeta,	noput},
/* PIO_LCD_BUSY */
    {NPIO_PA,	NPA_LCD_BUSY,	npiogeta,	noput},
/* PIO_LCD_DATA */
    {NPIO_PA,	0xff,		noget,		npioputa},
/* PIO_LCD_RW */
    {NPIO_PB,	NPB_LCD_RW,	softget,	npioputbc},
/* PIO_LCD_E */
    {NPIO_PB,	NPB_LCD_E,	softget,	npioputbc},
/* PIO_LCD_RS */
    {NPIO_PB,	NPB_LCD_RS,	softget,	npioputbc},
/* PIO_LED_NREADY */
    {NPIO_PB,	NPB_LED_NREADY,	softget,	npioputbc},
/* PIO_LED_NDATA */
    {NPIO_PB,	NPB_LED_NDATA,	softget,	npioputbc},
/* PIO_LED_NONLINE */
    {NPIO_PB,	NPB_LED_NONLINE,softget,	npioputbc},
/* PIO_E2_DIN */
    {NPIO_PC,	NPC_E2_DIN,	softget,	npioputbc},
/* PIO_E2_SK */
    {NPIO_PC,	NPC_E2_SK,	softget,	npioputbc},
/* PIO_E2_CS */
    {NPIO_PC,	NPC_E2_CS,	softget,	npioputbc},
/* PIO_E2_DOUT */
    {NPIO_PA,	NPA_E2_DOUT,	npiogetbc,	noput},
/* PIO_TEMP_2 */
    {NPIO_PC,	NPC_TEMP_2,	softget,	npioputbc},
/* PIO_TEMP_1 */
    {NPIO_PC,	NPC_TEMP_1,	softget,	npioputbc},
/* PIO_TEMP_0 */
    {NPIO_PC,	NPC_TEMP_0,	softget,	npioputbc},
/* PIO_PP_NSEL2 */
    {NPIO_PC,	NPC_PP_NSEL2,	softget,	npioputbc},
/* PIO_PP_NSEL1 */
    {NPIO_PC,	NPC_PP_NSEL1,	softget,	npioputbc},
/* PIO_ENGSEL */
    {NPIO_PA,	NPA_ENGSEL,	npiogetbc,	noput},
};



static const struct piomap * const piomaps[] = {
#define PIO_I8255	0
    i8255map, 
#define PIO_NPIO	1
    npiomap
};

/* translate pointer to uncached space */
#define ucptr(x)	((__typeof__(x))KVA0_TO_KVA1(x))

/* translate pointer to uncached space and dereference */
#define ucderef(x)	(*ucptr(x))

/* reference uncached object */
#define uc(x)		ucderef(&x)


#define piogetmap(port) ucptr(&(uc(piomaps[piotype])[(int)(port)]))

/*
 * Then generic interface to the pio
 */

void pio_init ()
{
    int type;
    if (i8255init ())
	type = PIO_I8255;
    else if (npioinit ())
	type = PIO_NPIO;
    else
	abort ();
    piotype = type;
}


unsigned int pio_put (enum pioport port, unsigned int val)
{
    const struct piomap *map;
    unsigned int oval, diff;

    map = piogetmap (port);
    oval = ucderef(map->get) (map);
    diff = (oval ^ val) & map->bits;
    if (diff != 0)
	ucderef(map->put) (map, oval ^ diff);
    return oval & map->bits;
}


unsigned int pio_get (enum pioport port)
{
    const struct piomap *map;
    unsigned int oval;

    map = piogetmap (port);
    oval = ucderef(map->get) (map);
    return oval & map->bits;
}


unsigned int pio_bis (enum pioport port)
{
    const struct piomap *map;
    unsigned int oval;

    map = piogetmap (port);
    oval = ucderef(map->get) (map);
    if ((oval & map->bits) == 0)
	ucderef(map->put) (map, oval | map->bits);
    return oval & map->bits;
}


unsigned int pio_bic (enum pioport port)
{
    const struct piomap *map;
    unsigned int oval;

    map = piogetmap (port);
    oval = ucderef(map->get) (map);
    if ((oval & map->bits) != 0)
	ucderef(map->put) (map, oval & ~map->bits);
    return oval & map->bits;
}
