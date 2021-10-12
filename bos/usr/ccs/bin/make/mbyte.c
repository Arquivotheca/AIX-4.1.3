#ifndef lint
static char sccsid[] = "@(#)19	1.2 src/bos/usr/ccs/bin/make/mbyte.c, cmdmake, bos411, 9428A410j 6/20/94 10:50:19";
#endif /* lint */
/*
 *   COMPONENT_NAME: CMDMAKE      System Build Facilities (make)
 *
 *   FUNCTIONS: Mbyte_search
 *		Mbyte_isupper_slow
 *		Mbyte_isspace_slow
 *		
 *
 *   ORIGINS: 27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*-
 * mbyte.c --
 *      Functions to do character handling on single- and multi-byte
 *	characters. Any invalid multi-byte characters are treated
 *	byte-by-byte as if they were single-byte characters.
 *
 * Interface:
 *      Mbyte_search            Search for a character in a single- or
 *				multi-byte string. Return 1 if character
 *				was found or 0 if not found.
 *
 *      Mbyte_isupper_slow           Check if a single- or multi-byte character
 *				is in uppercase. If character is single-byte
 *				isupper() is called. If character is
 *				multi-byte it is converted to a wide
 *				character and then iswupper() is called.
 *				Return 1 if character is uppercase, otherwise
 *				return 0.
 *
 *      Mbyte_isspace_slow	Check if a single- or multi-byte character
 *				is a space character (ie. space, tab, etc.).
 *				If character is single-byte isspace() is 
 *				called. If character is	multi-byte it is 
 *				converted to a wide character and then 
 *				iswspace() is called. Return 1 if character
 *				is uppercase, otherwise	return 0.
 *
 */

#include <stdlib.h>
#include "make.h"

extern int mb_cur_max;

/*-
 *-----------------------------------------------------------------------
 * Mbyte_search --
 *      Search for character in single- or multi-byte string.
 *
 * Results:
 *      1 if character in string, 0 if not.
 *
 * Side Effects:
 *      None.
 *-----------------------------------------------------------------------
 */
int
Mbyte_search(
        register char    *s,  /* MB character string to be checked */
        register uchar_t  c   /* character (single byte) */
	)
{
        register int    len;  /* length of MB character */

        while(*s) {
                if ((len = MBLENF(s)) > 1) {
                        s += len;
                } else {
                        if (*s++ == c) {
                                return (1);
                        }
                }
        }
        return (0);
}

/*-
 *-----------------------------------------------------------------------
 * Mbyte_isupper_slow --
 *	Check for upper case character in single- or multi-byte
 *	character string.
 *
 * Results:
 *      1 if character is uppercase, 0 if not.
 *
 * Side Effects:
 *      None.
 *-----------------------------------------------------------------------
 */
int
Mbyte_isupper_slow(
	const char	*s   /* MB character string to be checked */
	)
{
	int	result = 0;     /* Return status */

	if (MBLENF(s) > 1) {
		wint_t wc;   /* Wide character to be checked */

		mbtowc(&wc, s, mb_cur_max);
		if (iswupper(wc)) 
			result = 1;
	}
	else {
		if (isupper(*s))
			result = 1;
	}
	return(result);
}

/*-
 *-----------------------------------------------------------------------
 * Mbyte_isspace_slow --
 *	Check for space character in single- or multi-byte
 *	character string.
 *
 * Results:
 *      1 if character is a space character, 0 if not.
 *
 * Side Effects:
 *      None.
 *-----------------------------------------------------------------------
 */
int
Mbyte_isspace_slow(
	const	char	*s   /* MB character string to be checked */
	)
{
	int	result = 0;     /* Return status */

	if (MBLENF(s) > 1) {
		wint_t wc;   /* Wide character to be checked */

		mbtowc(&wc, s, mb_cur_max);
		if (iswspace(wc)) 
			result = 1;
	}
	else {
		if (isspace(*s))
			result = 1;
	}
	return(result);
}
