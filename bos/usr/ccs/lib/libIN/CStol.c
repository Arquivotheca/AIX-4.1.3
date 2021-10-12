static char sccsid[] = "@(#)99	1.7  src/bos/usr/ccs/lib/libIN/CStol.c, libIN, bos411, 9428A410j 3/30/94 16:05:17";
/*
 * LIBIN: CStol
 *
 * ORIGIN: 9
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1994
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: Convert string to long number.
 *
 * RETURN VALUE DESCRIPTION: 
 */

#define _ILS_MACROS
#include <ctype.h>
#include <IN/standard.h>

long
CStol(str, newp, base)
 char *str, **newp;
 register base;
{
	register unsigned c;
	register char *cp;
	register long n = 0;
	register sign;

	if( (cp = str) == NULL )
	    goto ret;

	while( isspace(c = *cp) )
	    cp++;
	cp++;		/* move cp to 2nd non-space character */
			/* while c holds the 1st one 	      */
	sign = c;
	if( c == '-' || c == '+' )
	    c = *cp++;

	if( c == '0' )
	{   c = *cp++;
	    if( (c == 'x' || c == 'X') && (base == 0 || base == 16) )
	    {   base = 16;
		c = *cp++;
	    }
	    if( base == 0 )
		base = 8;
	}
	if( base == 0 )
	    base = 10;

	for( ;; c = *cp++ )
	{   if( !isdigit(c) )
	    {   if( !isalpha(c) )
		    break;
		if( isupper(c) )
		    c += 'a' - 'A';
		c += '0' + 10 - 'a';
	    }
	    c -= '0';
	    if( c >= base )
		break;
	    n = n*base + c;
	}
	--cp;
	if( sign == '-' )
	    n = -n;
ret:
	if( newp )
	    *newp = cp;
	return n;
}
