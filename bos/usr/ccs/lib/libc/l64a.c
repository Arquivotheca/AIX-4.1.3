static char sccsid[] = "@(#)51	1.5  src/bos/usr/ccs/lib/libc/l64a.c, libccnv, bos411, 9428A410j 1/20/94 14:36:51";
/*
 * COMPONENT_NAME: LIBCCNV l64a
 *
 * FUNCTIONS: l64a, l64a_r
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <values.h>
#ifdef _THREAD_SAFE
#include <stdlib.h>		/* l64a_r */
#include <errno.h>		/* errno, EINVAL */
#endif /* _THREAD_SAFE */

#define BITSPERCHAR	6 /* to hold entire character set */
#define BITSPERLONG	(BITSPERBYTE * sizeof(long))
#define NMAX		((BITSPERLONG + BITSPERCHAR - 1)/BITSPERCHAR)
#define SIGN		(-(1L << (BITSPERLONG - BITSPERCHAR - 1)))
#define CHARMASK	((1 << BITSPERCHAR) - 1)
#define WORDMASK	((1L << ((NMAX - 1) * BITSPERCHAR)) - 1)

#ifndef _THREAD_SAFE

/*
 * NAME: l64a
 *                                                                    
 * FUNCTION: convert long int to base 64 ascii
 *                                                                    
 * NOTES:
 * convert long int to base 64 ascii
 * char set is [./0-9A-Za-z]
 * two's complement negatives are assumed,
 * but no assumptions are made about sign propagation on right shift
 *
 * RETURNS: a character string
 */

static char __buf[NMAX + 1];

char *
l64a(lg)
register long lg;
{
	register char *s = __buf;

	while (lg != 0) {

		register int c = ((int)lg & CHARMASK) + ('0' - 2);

		if (c > '9')
			c += 'A' - '9' - 1;
		if (c > 'Z')
			c += 'a' - 'Z' - 1;
		*s++ = c;
		/* fill high-order CHAR if negative */
		/* but suppress sign propagation */
		lg = ((lg < 0) ? (lg >> BITSPERCHAR) | SIGN :
			lg >> BITSPERCHAR) & WORDMASK;
	}
	*s = '\0';
	return (__buf);
}

#else /* _THREAD_SAFE */

/*
 * NAME: l64a_r
 *                                                                    
 * FUNCTION: Convert long int to base 64 ascii in user's provided buffer.
 *
 * ARGUMENTS:
 *	lg	The long int to convert.  The value 0L will convert to "".
 *	__buf	The place to put the base-64 string; char __buf[len];
 *		This buffer includes the trailing nul char.
 *	len	The size of the output buffer.
 *
 * RETURNS: an int:
 *  0 == success, all OK
 * -1 == failure, no buffer, or buffer too small
 *                                                                    
 * NOTES:
 * convert long int to base 64 ascii
 * char set is [./0-9A-Za-z]
 * two's complement negatives are assumed,
 * but no assumptions are made about sign propagation on right shift
 *
 * ASSERTIONS:
 *  1 __buf is NULL => returns -1 and errno == EINVAL.
 *  2 len is too small => returns -1 and errno == EINVAL.
 *  3 The value 0 results in an empty string in a valid buffer, ret == 0.
 *  4 The '.'       represents 0 (zero).
 *  5 The '/'       represents 1.
 *  6 The '0' - '9' represents 2 to 11.
 *  7 The 'A' - 'Z' represents 12 to 37.
 *  8 The 'a' - 'z' represents 38 to 63.
 *  9 Least significant 'digit' is first in valid buffer.
 * 10 l64a_r produces at most 6 characters plus a trailing '\0'.
 * 11 Prototype for l64a_r is in stdlib.h.
 * 12 l64a_r's object code is in libc_r.a.
 * 13 l64a_r is reentrant.
 *
 * These requirements come from:
 * 1) OSF/1 Programmers's Reference Revision 1.1 May 22, 1992, page 1-13, a64l entry.
 * 2) X/Open UU/COSE November 24, 1993 a64l entry.
 */

int
l64a_r( long lg, char * __buf, int len ){
	register char *s = __buf;	/* where store next char */
	int done = 0;	/* number of characters stored */

	if( (__buf==NULL) || (len<1) ) goto death;

	while (lg != 0) {

		register int c = ((int)lg & CHARMASK) + ('0' - 2);

		if (c > '9')
			c += 'A' - '9' - 1;
		if (c > 'Z')
			c += 'a' - 'Z' - 1;
		*s++ = c;
		done++;		/* have output another char */
		if( done == len ) goto death;	/* no room for '\0' */
		
		/* fill high-order CHAR if negative */
		/* but suppress sign propagation */
		lg = ((lg < 0) ? (lg >> BITSPERCHAR) | SIGN :
			lg >> BITSPERCHAR) & WORDMASK;
	}
	*s = '\0';
	return (0);	/* success return */

    death:
	errno = EINVAL;	/* indicate something invalid */
	return (-1);	/* failure return */
	
}

#endif /* _THREAD_SAFE */
