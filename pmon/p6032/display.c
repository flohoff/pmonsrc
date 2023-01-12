/*
 * p6032/display.c: low-level routines to talk to the LED front panel display
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
#include <pmon.h>

#include "sbd.h"
#include "hd2532.h"

static void
led_message (const char *str) 
{
    volatile unsigned int *led = PA_TO_KVA1 (LED_BASE + HD2532_CRAM);
    int i;

    /* ignore row */
    for (i = 0; i < 8; i++) {
	if (*str)
	    led[i] = *str++;
	else
	    break;
    }
}


static void
led_flash (int on)
{
    volatile unsigned int *ledcw = PA_TO_KVA1 (LED_BASE) + HD2532_CW;
    if (on)
	*ledcw = (*ledcw & ~HD2532_CW_BMASK) | HD2532_CW_B100;
    else
	*ledcw = (*ledcw & ~HD2532_CW_BMASK) | HD2532_CW_B0;
}


void display_fatal (const char *line1, const char *line2)
{
    led_message (line1);
}


void display_message (int line, const char *str)
{
#if 0
    const char *ns = str;
    while (*ns == ' ') ns++; /* skip initial spaces */
#endif
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
    led_message ("        ");
}


void display_size (int *prows, int *pcols)
{
    /* 1 x 8 LED display */
    if (prows) *prows = 1;
    if (pcols) *pcols = 8;
}


int display_init (int *prows, int *pcols)
{
    volatile unsigned int *ledcw = PA_TO_KVA1 (LED_BASE) + HD2532_CW;

    *ledcw = HD2532_CW_C | HD2532_CW_B100;
    usdelay (110);

    return 1;
}
