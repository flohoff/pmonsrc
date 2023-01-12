/*
 * ./strtol.c : stdlib function
 * Copyright (c) 1992 Algorithmics Ltd.
 */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>

#define LONG_MAX 	2147483647L
#define LONG_MIN 	(-LONG_MAX-1)

long int strtol(const char *nptr,char **endptr,int base)
{
    int c;
    long result = 0L;
    long limit;
    int negative = 0;
    int overflow = 0;
    int digit;

    while ((c = *nptr) && isspace(c)) /* skip leading white space */
      nptr++;
    if ((c = *nptr) == '+' || c == '-') { /* handle signs */
	negative = (c == '-');
	nptr++;
    }
    if (base == 0) {		/* determine base if unknown */
	base = 10;
	if (*nptr == '0') {
	    base = 8;
	    nptr++;
	    if ((c = *nptr) == 'x' || c == 'X') {
		base = 16;
		nptr++;
	    }
	}
    }
    else
      if (base == 16 && *nptr == '0') {	/* discard 0x/0X prefix if hex */
	  nptr++;
	  if ((c = *nptr == 'x') || c == 'X')
	    nptr++;
      }

    limit = LONG_MAX / base;	/* ensure no overflow */

    nptr--;			/* convert the number */
    while (c = *++nptr) {
	if (isdigit(c))
	  digit = c - '0';
	else if(isalpha(c))
	  digit = c - (isupper(c) ? 'A' : 'a') + 10;
	else
	  break;
	if (digit < 0 || digit >= base)
	  break;
	if (result > limit)
	  overflow = 1;
	if (!overflow) {
	    result = base * result;
	    if (digit > LONG_MAX - result)
	      overflow = 1;
	    else	
	      result += digit;
	}
    }
    if (negative && !overflow)
      result = 0L - result;
    if (overflow) {
	errno = ERANGE;
	if (negative)
	  result = LONG_MIN;
	else
	  result = LONG_MAX;
    }

    if (endptr != NULL)		/* set up return values */
      *endptr = (char *)nptr;
    return result;
}
