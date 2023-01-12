/* $Id: getchar.c,v 1.2 1996/01/16 14:18:00 chris Exp $ */
#include <stdio.h>

/*************************************************************
 *  getchar() get char from stdin
 */
getchar ()
{

    return (getc (stdin));
}
