/* $Id: eraline.c,v 1.2 1996/01/16 14:17:47 chris Exp $ */
/*************************************************************
 *  era_line(s) erase line
 */
era_line (s)
     char           *s;
{
    while (*s++)
	printf ("\b \b");
}
