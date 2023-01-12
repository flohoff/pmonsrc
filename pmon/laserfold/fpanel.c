/* $Id: fpanel.c,v 1.2 1997/02/20 15:02:23 nigel Exp $ */
/*
 * fpanel.c: routines to handle the front-panel display
 */

#include <mips.h>
#include <pmon.h>
#include <sbd.h>
#include <fpanel.h>
#include <pio.h>

static inline int
lcd_busy ()
{
    int busy;
    pio_get (PIO_LCD_BUSY); 	/* turn data buffers around */
    pio_bic (PIO_LCD_RS);
    pio_bis (PIO_LCD_RW);

    /* toggle clock and sample busy line in middle */
    sbddelay (1);
    pio_bis (PIO_LCD_E);
    /* max data delay 320ns; min E pulse 450ns */
    busy = pio_get (PIO_LCD_BUSY);
    busy = pio_get (PIO_LCD_BUSY);
    pio_bic (PIO_LCD_E);

    return !!busy;
}


static int
lcd_wait ()
{
    int timeout = 2000;		/* ~2ms */
    int busy;

    do {
	busy = lcd_busy ();
    } while (busy && --timeout != 0);
    sbddelay (1);
    return !busy;
}


static void
lcd_command (unsigned char cmd) 
{
    if (!lcd_wait ())
	return;

    /* prepare command and direction */
    pio_bic (PIO_LCD_RS);
    pio_bic (PIO_LCD_RW);
    pio_put (PIO_LCD_DATA, cmd);

    /* toggle enable clock */
    sbddelay (1);
    pio_bis (PIO_LCD_E);
    sbddelay (1);
    pio_bic (PIO_LCD_E);

    /* normal command cycle time = 40us */
    sbddelay (40);
}


static void
lcd_data (unsigned char data) 
{
    if (!lcd_wait ())
	return;

    /* prepare data and direction */
    pio_bis (PIO_LCD_RS);
    pio_bic (PIO_LCD_RW);
    pio_put (PIO_LCD_DATA, data);

    /* toggle enable clock */
    sbddelay (1);
    pio_bis (PIO_LCD_E);
    sbddelay (1);
    pio_bic (PIO_LCD_E);

    /* data write cycle time = 40us */
    sbddelay (40);
}


void
lcd_display (int line, const char *s)
{
    int twoline = pio_get (PIO_ENGSEL);
    int n;

    if (line == 1 && twoline)
	lcd_command (LCD_SET_DDADDR | 0x40);
    else
	lcd_command (LCD_SET_DDADDR | 0x00);

    for (n = 0; n < 16; n++) {
	if (!twoline && n == 8)
	    /* jump to second line */
	    lcd_command (LCD_SET_DDADDR | 0x40);
	if (*s)
	    lcd_data (*s++);
	else
	    lcd_data (' ');
    }
}

void
fpanel_init (void)
{
    /* READY & DATA off; ONLINE on*/
    pio_bis (PIO_LED_NREADY);
    pio_bis (PIO_LED_NDATA);
    pio_bic (PIO_LED_NONLINE);

    /* program display */
    lcd_command (LCD_FUNC | LCD_FUNC_8BIT | LCD_FUNC_2LINE | LCD_FUNC_5x7);
    lcd_command (LCD_CTRL | LCD_CTRL_DISPLAY);
    lcd_command (LCD_MODE | LCD_MODE_INCR);
    lcd_command (LCD_CLEAR);

    {
	char msg[16];
	strcpy (msg, "PMON ");
	strcat (msg, vers);
	lcd_display (1, msg);
    }
}
