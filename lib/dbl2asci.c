/* $Id: dbl2asci.c,v 1.2 1996/01/16 14:17:47 chris Exp $ */

#define strequ(x,y)	((strcmp(x,y)==0)?1:0)

dbl_to_ascii (in, dst)
     double         *in;
     char            dst[];
{
    int             i;
    char            tmp1[64], tmp2[64];
    long            exp;
    char            sign;

#ifdef FLOATINGPT
    double          d;

/* dtoa clobbers 'in' so copy it first */
    d = *in;
    dtoa (&d, tmp1, &sign, &exp);

    *dst = 0;
    if (sign == '-')
	strcat (dst, "-");

/* mantissa */
    if (strequ (tmp1, "0"))
	sprintf (tmp2, "0.%s", tmp1);
    else {
	sprintf (tmp2, "%c.%s", tmp1[0], &tmp1[1]);
	exp--;
    }
    for (i = strlen (tmp2); i < 25; i++)
	tmp2[i] = '0';
    tmp2[i] = 0;
    strcat (dst, tmp2);

/* exponent */
    sprintf (tmp1, "%02d", exp);
    if (tmp1[0] == '-')
	strcat (dst, "e");
    else
	strcat (dst, "e+");
    strcat (dst, tmp1);
#endif /* FLOATINGPT */
}
