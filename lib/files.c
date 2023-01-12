/* $Id: files.c,v 1.2 1996/01/16 14:17:54 chris Exp $ */
#include <termio.h>

/*************************************************************
 *  _mfile[]
 *      A stub to satisfy the linker when building PMON. This is
 *      necessary because the same read() and write() routines are
 *      used by both PMON and the client, and the client supports
 *      ram-based files.
 */

Ramfile         _mfile[] =
{
    {0}};
