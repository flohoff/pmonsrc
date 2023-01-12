/*
 * P4032/display.c: low-level routines to talk to the LCD front panel display
 */

#include <mips.h>
#include <pmon.h>

#include "sbd.h"

void
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


void
led_flash (int on)
{
    if (on)
	bcr_bis (BCR_LED_ON);
    else
	bcr_bic (BCR_LED_ON);
}


void display_fatal (const char *line1, const char *line2)
{
}


void display_message (int line, const char *str)
{
    led_message (str);
}


void display_progress (int percent)
{
}


void display_flash (int on)
{
    led_flash (on);
}


void display_clear ()
{
    led_message ("    ");
}


void display_size (int *prows, int *pcols)
{
    /* 1 x 4 LED display */
    if (prows) *prows = 1;
    if (pcols) *pcols = 4;
}


int display_init (void)
{
    return 1;
}
