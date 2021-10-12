static char sccsid[] = "@(#)29	1.15  src/bos/usr/ccs/lib/libc/gcvt.c, libccnv, bos411, 9428A410j 1/30/94 12:42:12";
/*
 * COMPONENT_NAME: LIBCCNV gcvt
 *
 * FUNCTIONS: gcvt
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

#include <string.h>
#include <stdlib.h>		/* ecvt(), ecvt_r() */
#include <errno.h>		/* errno, EINVAL */
#include <math.h>
#include <fp.h>
#include <langinfo.h>		/* nl_langinfo, RADIXCHAR */

/* TRIMZERO causes trailing zeros to be removed.  This is
 * required for bsd compatability.  The #ifdef around this
 * code has been around since sccs level 1.1, so it came
 * with the code 
 */

#define TRIMZERO
#define BSIZE 31	/* ecvt can do at most 30 digits + trailing null. */

/*
 * NAME: gcvt
 *                                                                    
 * FUNCTION: convert double to readable ascii string
 *                                                                    
 * NOTES:
 *
 * RETURNS: a character string
 *
 */

char *
gcvt( double number, int ndigit, char *buf )
{
	int sign, decpt;
	register char *p1, *p2 = buf;
	register int i;
	char *radix;	        /* radix character for current locale */
	char *radix_end;	/* pointer to last char in radix */
	int rsize = 0;		/* number of chars in radix */
	int nd;			/* number of digits ecvt will produce: 1...30 */
#ifdef TRIMZERO
	register char *p;	/* walks thru work_buf or ecvt's buf */
#endif
#ifdef _THREAD_SAFE
	int rc;			/* return code from ecvt_r */
	char work_buf[BSIZE];	/* buffer for ecvt_r */
#endif /* _THREAD_SAFE */
	
	if( (NULL == buf)	/* make sure have a place to store */
	 || (ndigit < 1) ){	/* and valid number of digits */
	    errno = EINVAL;	/* indicate an invalid argument */
	    return( buf );	/* and return that bad ptr */
	}
	/* 
	 * Limit what we ask ecvt to do, since everything beyond BSIZE-1
	 * will just be zeros, and our buffer is BSIZE big.
	 */
	if( ndigit < BSIZE )
	    nd = ndigit;	/* already checked that ndigit > 0 */
	else
	    nd = BSIZE-1;
	/* 
	 * 0 < nd < BSIZE.  work_buf[nd] will be '\0'
	 */
#ifdef _THREAD_SAFE
	rc = ecvt_r( number, nd, &decpt, &sign, work_buf, BSIZE );
	if( rc ){		/* ecvt_r had a problem; it set errno */
	    *buf = '\0';	/* make result buffer be empty */
	    return( buf );	/* return that empty buffer */
	}
	p1 = work_buf;		/* where ecvt_r put its result */
#else /* regular case */
	p1 = ecvt( number, nd, &decpt, &sign );
#endif /* _THREAD_SAFE */
	if (sign)
		*p2++ = '-';
	/* If string returned from ecvt is not numeric then it should	*/
	/* 		be "NaNQ", "NaNS", or "INF"			*/
	if( !FINITE(number) ) {	/* ISSUE: what if ndigits is < 4? */
		strcpy(p2,p1);	/* ISSUE: assumes user's buffer is big enough */
		return(buf);
	}

	/* Get radix from current locale.  If it comes back a null string
	 * or null pointer, use default radix (full stop).  We assume that
	 * radix character may have multiple characters.  For this implementation,
	 * the radix must NOT contain a character that can be otherwise
	 * meaningful in a decimal string:  +, -, e, 0-9
	 * We emit the radix in a while loop, and count the number of characters
	 * emitted and keep a pointer to the last chracter emitted.
	 */
	radix = nl_langinfo(RADIXCHAR);
	if ( (radix == NULL) || !(*radix) )
		radix = ".";

#ifdef TRIMZERO
	/* Remove trailing zeros in our buffer before copy to user's buffer.
	 * This way, we do not overrun the user's buffer by copying too much.
	 * Leave the '0' at work_buf[0] if the entire buffer is 0's.
	 */
	for (p=p1+nd-1; (*p == '0') && (p > p1); p--)
	    *p = '\0';				/* replace trailing '0' with '\0' */
#endif

	if (decpt > ndigit || decpt <= -4) {	/* E-style */
		decpt--;
		*p2++ = *p1++;			/* first digit */
#ifdef TRIMZERO
		if ( *p1 )			/* copy radix only if more digits */
#endif
		while (*radix != '\0') {
		        radix_end = radix;
			*p2++ = *radix++;
			rsize++;
		}
		for (i = 2; i <= ndigit; i++)	/* digits 2...ndigit */
		    if( *p1 ){			/* have a valid digit to copy */
			*p2++ = *p1++;
#ifndef TRIMZERO
		    }else{			/* hit null, so copy '0's */
			*p2++ = '0';
#endif
		    }
		
		*p2++ = 'e';
		if (decpt < 0) {
			decpt = -decpt;
			*p2++ = '-';
		} else
			*p2++ = '+';
		for (i = 1000; i != 0; i /= 10) /* 3B or CRAY, for example */
			if (i <= decpt || i <= 10) /* force 2 digits */
				*p2++ = (decpt / i) % 10 + '0';
	} else {				/* F-style */
		if (decpt <= 0) {		/* form will be 0.0...0nnnn  or 0.nnnn */
			*p2++ = '0';
#ifdef TRIMZERO
			if ( '0' == *p1 ){	/* the value zero is special (0, not 0.0) */
			    *p2 = '\0';
			    return (buf);
			}
#endif
			while (*radix != '\0') {
		        	radix_end = radix;
				*p2++ = *radix++;
				rsize++;
			}

			while (decpt < 0) {
				decpt++;
				*p2++ = '0';
			}
		}

		for (i = 1; i <= ndigit; i++) {
		        if( *p1 ){		/* have a valid digit to copy */
			    *p2++ = *p1++;
			}else{			/* hit null, so copy '0's */
#ifdef TRIMZERO
			    if( i <= decpt )	/* before radix */
#endif
				*p2++ = '0';
			}
			if( (i == decpt) 
#ifdef TRIMZERO
			    && ( *p1 )		/* copy radix only if more digits */
#endif
			){
				while (*radix != '\0') {
		        		radix_end = radix;
					*p2++ = *radix++;
					rsize++;
				}
			}
		}
	}
	*p2 = '\0';
	return (buf);
}

