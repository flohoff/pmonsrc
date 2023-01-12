/* $Id: demo.c,v 1.2 1996/01/16 14:23:01 chris Exp $ */
#include "mips.h"
#include "pmon.h"
#include "stdio.h"
#include "termio.h"

#undef SABLE

/*************************************************************
 *  demo.c
 *      This module can be used to include a demo program with
 *      PMON, 'hanoi' is one such demo.
 */

int             hanoi ();
int             sinewave ();
int             ftest ();
int             fptst ();
int             ldsym ();

Demo            demo[] =
{
#ifdef INCLUDE_HANOI
    {"hanoi", hanoi},
#endif				/* INCLUDE_HANOI */
#ifdef SABLE
    {"ldsym", ldsym},
#endif
#ifdef FLOATINGPT
#ifdef SABLE
    {"fptst", fptst},
#endif				/* SABLE */
#endif
    {0}};

demoinit ()
{
    int             i;

    for (i = 0; demo[i].name; i++) {
	newsym (demo[i].name, demo[i].addr);
    }
}

#ifdef FLOATINGPT
#ifdef SABLE
fptst ()
{
    float           f, x;
    double          y, z;

    x = 4.5678;
    y = 9.87654;
    z = 2.3456;

    f = x + y + z;
    if (f >= 16.7899 && f <= 16.7900)
	printf ("add test OK\n");
    else
	printf ("add test FAILED f=%e\n", f);

    f = x - y - z;
    if (f >= -7.65435 && f <= -7.65433)
	printf ("sub test OK\n");
    else
	printf ("sub test FAILED f=%e\n", f);

    f = x * y * z;
    if (f >= 105.819 && f <= 105.820)
	printf ("mult test OK\n");
    else
	printf ("mult test FAILED f=%e\n", f);

    f = x / y / z;
    if (f >= 0.197173 && f <= 0.197174)
	printf ("div test OK\n");
    else
	printf ("div test FAILED f=%e\n", f);
}
#endif
#endif

#ifdef SABLE
ldsym (adr)
     char           *adr;
{
    char            buf[LINESZ];
    int             c;

    for (;;) {
	c = getln (&adr, buf);
	if (c == 0)
	    break;
	write (1, ".", 1);
	do_cmd (buf);
    }
}
#endif
