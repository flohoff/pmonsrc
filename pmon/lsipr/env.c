/* $Id: env.c,v 1.2 1996/01/16 14:25:02 chris Exp $ */
/*
 * env.c: dummy nvram handler fo systems without nvram.
 */

#include "mips.h"
#include "pmon.h"
#include "string.h"


char *
sbd_getenv (name)
    char *name;
{
    return 0;
}


int
sbd_setenv (name, value)
    char *name, *value;
{
    return 0;
}


sbd_unsetenv (name)
    char *name;
{
    return 0;
}


sbd_mapenv (pfunc)
    int (*pfunc)();
{
    return 0;
}
