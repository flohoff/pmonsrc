/* $Id: sbrk.c,v 1.2 1996/01/16 14:23:18 chris Exp $ */
#include "pmon.h"

char           *getenv ();

#ifdef BSOTSUN
char           *allocp1 = 0;	/* this needs special code to set the value */
#else
extern char     end[];
char           *allocp1 = end;
char           *heaptop = 0;

#endif

int _pmon_in_ram;

int
chg_heaptop (name, value)
    char *name, *value;
{
    unsigned long top;

    if (atob (&top, value, 16)) {
	if (!_pmon_in_ram) {
	    if (K0_TO_PHYS(top) < K0_TO_PHYS(allocp1)) {
		printf ("%x: heap is already above this point\n", top);
		return 0;
	    }
	    heaptop = (char *) top;
	}
	return 1;
    }
    printf ("%s: invalid address\n", value);
    return 0;
}


char           *
sbrk (n)
     int             n;
{
    char            *top;

    top = heaptop;
    if (!top)
      top = (char *) CLIENTPC;

    if (K0_TO_PHYS(allocp1) + n <= K0_TO_PHYS(top)) {
	allocp1 += n;
	return (allocp1 - n);
    }

    return (0);
}
