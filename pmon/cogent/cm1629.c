/* $Id: cm1629.c,v 1.2 1996/01/16 14:24:31 chris Exp $ */
/*
 * fpanel.c: routines to handle the front-panel display
 */

#include <mips.h>
#include <pmon.h>
#include <sbd.h>
#include <cm1629.h>

static int fpanel_missing;

#define BUSYGLITCH
static int
lcd_wait ()
{
    volatile cm1629dev *dp = (volatile cm1629dev *)PHYS_TO_K1(LCD_BASE);
    int timeout = 4000;		/* ~2ms */
    int busy;

#ifdef BUSYGLITCH
    int idles = 0;
#endif
    do
	{
	busy = dp->lcd_stat & LCD_STAT_BUSY;
#ifdef BUSYGLITCH
	if (busy)
	    idles = 0;
	else
	    {
	    /* It's idle (apparently), but fake busy until
	     * we see it twice in a row
	     */
	    if (++idles < 2)
		busy = 1;
	    }
#endif
	} while (busy && --timeout != 0);
    return !busy;
}


static int
lcd_command (unsigned char cmd) 
{
    volatile cm1629dev *dp = (volatile cm1629dev *)PHYS_TO_K1(LCD_BASE);
    if (!lcd_wait ())
	return;
    dp->lcd_cmd = cmd;
    wbflush ();
}


static int
lcd_data (unsigned char data) 
{
    volatile cm1629dev *dp = (volatile cm1629dev *)PHYS_TO_K1(LCD_BASE);
    if (!lcd_wait ())
	return;
    dp->lcd_data = data;
    wbflush ();
}


void
lcd_display (int line, const char *s)
{
    int n;

    if (fpanel_missing)
	return;

    if (line == 1)
	lcd_command (LCD_SET_DDADDR | 0x40);
    else
	lcd_command (LCD_SET_DDADDR | 0x00);

    for (n = 0; n < 16; n++) {
	if (*s)
	    lcd_data (*s++);
	else
	    lcd_data (' ');
    }
}

void
fpanel_init (void)
{
    if (!lcd_wait ()) {
	fpanel_missing = 1;
	return;
    }

    lcd_command (LCD_FUNC | LCD_FUNC_8BIT | LCD_FUNC_2LINE | LCD_FUNC_5x7);
    lcd_command (LCD_CTRL | LCD_CTRL_DISPLAY);
    lcd_command (LCD_MODE | LCD_MODE_INCR);
    lcd_command (LCD_CLEAR);

    {
	char msg[16];
	lcd_display (1, "Algorithmics Ltd");
	sprintf (msg, "PMON %s", vers);
	lcd_display (0, msg);
    }
}

