/*
 * share/mon.c: simple integer-only, unbuffered printf direct to hardware
 *
 * Copyright (c) 1996-1999 Algorithmics Ltd - all rights reserved.
 * 
 * This program is NOT free software, it is supplied under the terms
 * of the SDE-MIPS License Agreement, a copy of which is available at:
 *
 *  http://www.algor.co.uk/algor/info/sde-license.pdf
 *
 * Any company which has obtained and signed a valid SDE-MIPS license
 * may use and modify this software internally and use (without
 * restrictions) any derived binary.  You may not, however,
 * redistribute this in whole or in part as source code, nor may you
 * modify or remove any part of this copyright message.
 */
 
#include <sys/types.h>
#include <stdarg.h>
#include <ctype.h>

#ifndef IN_PMON
#include "kit_impl.h"
#endif

#define SIGNED	0x01
#define ALLSIGN	0x02
#define LEFT	0x04
#define ALT	0x08
#define UPPER	0x10
#define ZEROPAD	0x20
#define SPACE	0x40
#define PREC	0x80


static size_t
mystrlen (const char *s)
{
    const char *e = s;
    while (*e)
	++e;
    return e - s;
}


static char *
convert (int val, unsigned int base, unsigned int prec, 
	 unsigned int width, unsigned int flags)
{
    const char *digits = (flags & UPPER) 
	? "0123456789ABCDEF" 
	: "0123456789abcdef";
    static char buf[20];
    unsigned int n;
    int i;

    /* quick limit check */
    if (prec > sizeof(buf) - 4)
	prec = sizeof(buf) - 4;
    if (width > sizeof(buf) - 1)
	width = sizeof(buf) - 1;

    /* get absolute value */
    if (flags & SIGNED && val < 0)
	n = -val;
    else
	n = val;

    i = sizeof(buf);
    buf[--i] = '\0';
#define fldwidth	(sizeof(buf) - i - 1)

    /* generate digit string backwards */
    while (n != 0) {
	buf[--i] = digits[n % base];
	n /= base;
    }

    /* expand to minimum precision with leading zeros */
    while (fldwidth < prec)
	buf[--i] = '0';

    if (flags & ZEROPAD) {
	if (flags & ALT && val != 0)
	    /* leave room for base prefix */
	    width -= (base == 8) ? 1 : (base == 16) ? 2 : 0;
	/* pad to minimum width with leading zeros */
	while (fldwidth < width)
	    buf[--i] = '0';
    }

    if (flags & ALT && val != 0) {
	/* add base prefix */
	if (base == 8)
	    buf[--i] = '0';
	else if (base == 16)
	    buf[--i] = (flags & UPPER) ? 'X' : 'x', buf[--i] = '0';
    }

    /* add sign prefix */
    if (flags & SIGNED && val < 0)
	buf[--i] = '-';
    else if (flags & ALLSIGN)
	buf[--i] = '+';
    else if (flags & SPACE)
	buf[--i] = ' ';

#undef fldwidth

    return (&buf[i]);
}


void
_mon_vprintf (const char *format, va_list ap)
{
    char obuf[128], *op = obuf;
    int c;
    unsigned int base;
    int arg;
    unsigned int notflag, flags, width, prec, len;
    char *s;

    /* gosh, a gcc nested function! */
    static void _vpputc (char c)
	{
	    if (op >= &obuf[sizeof(obuf) - 2]) {
		*op = '\0';
		_mon_puts (op = obuf);
	    }
	    *op++ = c;
	}

    while (1) {
	if ((c = *format++) == '\0')
	    break;

	if (c == '\n') {
	    _vpputc ('\r');
	    _vpputc ('\n');
	    continue;
	}

	if (c != '%') {
	    _vpputc (c);
	    continue;
	}

	flags = width = 0;
	prec = 1;

	/* handle initial flags */
	notflag = 0;
	while (!notflag) {
	    c = *format++;
	    switch (c) {
	    case '0':
		flags |= ZEROPAD;
		break;
	    case '-':
		flags |= LEFT;
		break;
	    case ' ':
		flags |= SPACE;
		break;
	    case '+':
		flags |= ALLSIGN;
		break;
	    case '#':
		flags |= ALT;
		break;
	    default:
		notflag = 1;
		break;
	    }
	}

	if (flags & LEFT)
	    flags &= ~ZEROPAD;

	/* handle width */
	if (c == '*') {
	    width = va_arg (ap, int);
	    c = *format++;
	}
	else {
	    while (isdigit (c)) {
		width = width * 10 + c - '0';
		c = *format++;
	    }
	}

	/* handle precision */
	if (c == '.') {
	    flags |= PREC;
	    c = *format++;
	    if (c == '*') {
		prec = va_arg (ap, unsigned int);
		c = *format++;
	    }
	    else {
		prec = 0;
		while (isdigit (c)) {
		    prec = prec * 10 + c - '0';
		    c = *format++;
		}
	    }
	}

	/* handle size modifiers (redundant) */
	if (c == 'l' || c == 'h')
	    c = *format++;

	if (c == '\0')
	    break;

	switch (c) {
	case 'p':
	    flags |= ALT;
	    if (!(flags & PREC))
		prec = sizeof(void *)/4;
	    /* drop through */
	case 'X':
	    flags |= UPPER;
	    /* drop through */
	case 'x':
	    base = 16;
	    goto doconvert;
	case 'o':
	    base = 8;
	    goto doconvert;
	case 'd':
	case 'i':
	    flags |= SIGNED;
	    /* drop through */
	case 'u':
	    base = 10;
	    goto doconvert;
	doconvert:
	    /* XXX note assume short=int=long; don't handle "long long" */
	    arg = va_arg(ap, int);
	    s = convert (arg, base, prec, width, flags);
	    prec = len = mystrlen (s);
	    goto dostring;
	case 's':
	    s = va_arg(ap, char *);
	    if (!s)
		s = "(null)";
	    len = mystrlen (s);
	    if (!(flags & PREC) || len < prec)
		prec = len;
	dostring:
	    if (len < width && !(flags & LEFT)) {
		/* pad on left with spaces */
		do {
		    _vpputc (' ');
		} while (++len < width);
	    }
	    /* write the result */
	    while (prec-- != 0)
		_vpputc (*s++);
	    /* pad on right with spaces */
	    while (len++ < width)
		_vpputc (' ');
	    break;
	case 'c':
	    arg = va_arg(ap, int);
	    _vpputc (arg);
	    break;
	default:
	    _vpputc (c);
	    break;
	}
    }

    if (op > obuf) {
	*op = '\0';
	_mon_puts (obuf);
    }
}


void
_mon_printf (const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    _mon_vprintf (fmt, ap);
    va_end(ap);
}
