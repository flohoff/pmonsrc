/*
 * P4032/display.c: low-level routines to talk to the LCD front panel display
 */

#include <mips.h>
#include <pmon.h>
#include <p4032/sbd.h>
#include <p4032/lcd.h>

static int	lcd_present;
static char	lcd_slider[17];


static inline int
lcd_busy ()
{
    volatile lcddev * dp = PA_TO_KVA1(LCD_BASE);
    sbddelay (1);
    return (dp->lcd_stat & LCD_STAT_BUSY) != 0;
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
    volatile lcddev *dp = PA_TO_KVA1(LCD_BASE);
    if (!lcd_wait ())
	return;
    dp->lcd_cmd = cmd;
    sbddelay (40);
}


static void
lcd_wdata (unsigned char data) 
{
    volatile lcddev *dp = PA_TO_KVA1(LCD_BASE);
    if (!lcd_wait ())
	return;
    dp->lcd_data = data;
    sbddelay (40);
}

static unsigned int
lcd_rdata (void)
{
    volatile lcddev *dp = PA_TO_KVA1(LCD_BASE);
    unsigned int data;

    lcd_wait ();
    data = dp->lcd_data & 0xff;
    sbddelay (40);
    return data;
}


static void
lcd_message (int row, const char *s)
{
    int addr, n;

    addr = (row == 0) ? 0 : 0x40;
    lcd_command (LCD_SET_DDADDR | addr);
    
    for (n = 0; n < 16; n++) {
	if (*s)
	    lcd_wdata (*s++);
	else
	    lcd_wdata (' ');
    }
}


static void
lcd_flash (int on)
{
    lcd_command (LCD_SET_DDADDR | 15);
    lcd_wdata (on ? '*' : ' ');
}


#if 0
static void
lcd_setcursor (int row, int col)
{
    int addr;
    
    if (col > 15) 
	col = 15;
    addr = (row == 0) ? col : (0x40 + col);
    lcd_command (LCD_SET_DDADDR | addr);
}
#endif


static void
led_message (const char *str) 
{
    volatile unsigned int *led = PA_TO_KVA1 (LED_BASE);
    int i;
    for (i = 3; i >= 0; i--) {
	if (*str)
	    led[i] = *str++;
	else
	    led[i] = ' ';
    }
}


static void
led_flash (int on)
{
    if (on)
	bcr_bis (BCR_LED_ON);
    else
	bcr_bic (BCR_LED_ON);
}


void display_fatal (const char *line1, const char *line2)
{
    if (lcd_present) {
	lcd_command (LCD_CLEAR);
	lcd_message (0, line1);
	lcd_message (1, line2);
    }
}


void display_message (int line, const char *str)
{
    if (lcd_present) {
	const char *ns = str;
	while (*ns == ' ') ns++; /* skip initial spaces */
	led_message (ns);
	lcd_message (line, str);
    }
    else {
	led_message (str);
    }
}


void display_progress (int percent)
{
    if (lcd_present) {
	static int lastval = -1;
	int val = (percent * 16 + 50) / 100;
	
	if (val < 0)
	    val = 0;
	else if (val > 16) 
	    val = 16;
	if (val != lastval) {
	    lcd_message (0, &lcd_slider[16 - val]);
	    lastval = val;
	}
    }
}


void display_flash (int on)
{
    led_flash (on);
    if (lcd_present)
	lcd_flash (on);
}


void display_clear ()
{
    led_message ("    ");
    if (lcd_present)
	lcd_command (LCD_CLEAR);
}


void display_size (int *prows, int *pcols)
{
    if (lcd_present) {
	/* 2 x 16 LCD display */
	if (prows) *prows = 2;
	if (pcols) *pcols = 16;
    }
    else {
	/* 1 x 4 LED display */
	if (prows) *prows = 1;
	if (pcols) *pcols = 4;
    }
}


int display_init (int *prows, int *pcols)
{
    volatile lcddev * dp = PA_TO_KVA1(LCD_BASE);
    static const unsigned char tpat[4] = {0xaa, 0x55, 0xff, 0x00};
    int i;

    /* notify via led display */
    led_message ("DISP");

    /* should be non-busy (possibly after a delay) */
    lcd_present = lcd_wait ();
    if (!lcd_present)
	return 1;

    lcd_command (LCD_CLEAR);

    /* should be busy */
    lcd_present = lcd_busy();
    if (!lcd_present)
	return 1;

    /* initialise display as per our requirements */
    lcd_command (LCD_FUNC | LCD_FUNC_8BIT | LCD_FUNC_2LINE | LCD_FUNC_5x7);
    lcd_command (LCD_CTRL | LCD_CTRL_DISPLAY);
    lcd_command (LCD_MODE | LCD_MODE_INCR);
    lcd_command (LCD_HOME); 

    /* store pattern into display memory */
    lcd_command (LCD_SET_DDADDR | 0); 
    for (i = 0; i < sizeof(tpat); i++)
	lcd_wdata (tpat[i]); 

    /* read back pattern */
    lcd_command (LCD_SET_DDADDR | 0); 
    for (i = 0; i < sizeof(tpat); i++) {
	/* check dd ram contents */
	if (lcd_rdata () != tpat[i]) {
	    lcd_present = 0;
	    return 1;
	}
    }

    /* it really is there! */
    lcd_command (LCD_CLEAR);

    memset (lcd_slider, 0xff, sizeof(lcd_slider) - 1);
    lcd_slider[sizeof(lcd_slider)-1] = '\0';

    return 1;
}
