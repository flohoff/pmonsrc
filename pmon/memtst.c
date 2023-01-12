/* $Id: memtst.c,v 1.3 1997/06/16 11:30:48 chris Exp $ */
#include "mips.h"
#include "stdio.h"
#include "termio.h"
#include "pmon.h"

const Optdesc         mt_opts[] =
{
    {"-c", "continuous test"},
    {0}};

memtst (ac, av)
     int             ac;
     char           *av[];
{
    int             i, j, cnt, cflag, err;
    unsigned long   adr, siz;

    cflag = 0;
    cnt = 0;
    adr = PHYS_TO_K1 (K0_TO_PHYS (CLIENTPC));
    siz = memorysize - K1_TO_PHYS (adr);

    for (i = 1; i < ac; i++) {
	if (av[i][0] == '-') {
	    for (j = 1; av[i][j]; j++) {
		if (av[i][j] == 'c')
		    cflag = 1;
		else {
		    printf ("%c: bad option\n", av[i][j]);
		    return (-1);
		}
	    }
	} else {
	    switch (cnt) {
	    case 0:
		if (!get_rsa (&siz, av[i]))
		    return (-1);
		cnt++;
		break;
	    case 1:
		adr = siz;
		if (!get_rsa (&siz, av[i]))
		    return (-1);
		cnt++;
		break;
	    default:
		printf ("%s: Unknown argument\n", av[i]);
		return (-1);
	    }
	}
    }

/* make sure the address is word aligned */
    adr = adr & ~0x3;

    siz &= ~0x3;		/* make sure it's an exact number of words */

    ioctl (STDIN, CBREAK, NULL);

    printf ("Testing %08x to %08x %s\n", adr, adr + siz, (cflag) ? "continuous" : "");
    printf ("Memory test running..  ");

    if (cflag)
	while (!(err = domt (adr, siz)));
    else
	err = domt (adr, siz);

    if (err) {
	printf ("\b\nThere were %d errors.\n", err);
	return (1);
    }
    printf ("\b\nTest passed with no errors.\n");
    return (0);
}

domt (adr, siz)
     unsigned int   *adr;
     int             siz;
{
    int             i, j, err, temp;
    unsigned int    w, *p, r;

    err = 0;

/* walking ones test */
    for (p = adr, i = 0; i < siz; i += 4, p++) {
	w = 1;
	for (j = 0; j < 32; j++) {
	    *p = w;
	    temp = 0;
	    r = *p;
	    if (r != w) {
		err++;
		printf ("\b\nerror: addr=%08x read=%08x expected=%08x  ",
			p, r, w);
	    }
	    w <<= 1;
	    dotik (1, 0);
	}
    }

/* store the address in each address */
    for (p = adr, i = 0; i < siz; i += 4, p++) {
	*p = (unsigned int)p;
	dotik (1, 0);
    }

/* check that each address contains its address */
    for (p = adr, i = 0; i < siz; i += 4, p++) {
	r = *p;
	if (r != (unsigned int)p) {
	    err++;
	    printf ("\b\nerror: adr=%08x read=%08x expected=%08x  ", p, r, p);
	}
	dotik (1, 0);
    }
    return (err);
}
