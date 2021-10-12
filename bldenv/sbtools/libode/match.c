/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: amatch
 *		gmatch
 *		umatch
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 */
/*
 * HISTORY
 * $Log: match.c,v $
 * Revision 1.3.7.1  1993/11/08  20:18:12  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  20:17:30  damon]
 *
 * Revision 1.3.5.2  1993/04/28  14:35:54  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/28  14:34:42  damon]
 * 
 * Revision 1.3.2.3  1992/12/03  17:21:13  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:25  damon]
 * 
 * Revision 1.3.2.2  1992/09/24  19:01:52  gm
 * 	CR282: Made more portable to non-BSD systems.
 * 	[1992/09/23  18:22:08  gm]
 * 
 * Revision 1.3  1991/12/05  21:05:19  devrcs
 * 	Changed this so it no longer is a simple as the match used by find.
 * 	It now understands "!" and can handle "\" to allow special characters
 * 	to be used literally.
 * 	[91/01/08  12:15:38  randyb]
 * 
 * 	Put in libsb.a.
 * 	[90/12/12  09:28:25  damon]
 * 
 * $EndLog$
 */
/******************************************************************************
**                          Open Software Foundation                         **
**                              Cambridge, MA                                **
**                               April 1990                                  **
*******************************************************************************
**
**    Description:
**	This is the match functions for library libsb.a.
**	It determines if a string matches a pattern.  It supports the
**	following wild cards:
**	  * - match anything
**	  ? - match one occurance of anything
**	  [] - match any character in brackets
**	  [N1-N2] - match any charcter from N1 to N2
**	  ! - return the inverse results of the match
**	  \ - take the next character literally
**	  {} - ?????
**
*/

#ifndef lint
static char sccsid[] = "@(#)91  1.1  src/bldenv/sbtools/libode/match.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:25";
#endif /* not lint */

#include <string.h>
#include <ode/util.h>

int
gmatch (const char * s, const char * p)

	/*
	 * The following match routine was taken from the "find" command.
	 * The "!" option was added to support "not".  The "\" was
	 * added to support handling the special characters literally.
	 * Returns true if string 's' is matched by pattern 'p' where 'p'
	 * is a string with wildcards in it.
	 */

{
  if ( *p != '!' )
	return amatch(s, p);
  else
	return ( ! amatch(s, ++p));
}

int
amatch(const char * s, const char * p)
{
	register cc;
	int scc, k;
	int c, lc;
	char *scopy;
	char schar;
	char *endbrace;
	char *endcomma;

    scc = *s;
    lc = 077777;

    switch (c = *p) {
	case '{':
		k = 0;
		endbrace = strchr(p, '}');
		if (endbrace == (char *) 0) return 0;	/* No closing '}' */
		schar = *(scopy = (char *)s);

		while ((cc = *++p)) {
			if (cc == ',' || cc == '}') {
				k |= amatch(scopy, endbrace + 1);
				if (k) return 1;
				if (cc == '}') return 0;
				schar = *(scopy = (char *)s);/* Retry source */
			}
			else
			if (cc == schar) {
				/* Char match succeeded */
				schar = *++scopy;
			} else {
				/* Char match failed */
				endcomma = strchr(p, ',');
				if (endcomma == (char *) 0 ||
				    endcomma >= endbrace)
					return 0;

				p = endcomma;	/* Next choice */
				schar = *(scopy = (char *)s);/* Retry source */
			}
		}
		return 0;

	case '[':
		k = 0;

		while ((cc = *++p)) {
		  switch (cc) {
		    case ']':
		      if (k)
			return(amatch(++s, ++p));
		      else
			return(0);

		    case '-':
		      cc = p[1];
		      k |= lc <= scc && scc <= cc;
		      break;

		    case '\\':
		      cc = *++p;
		  } /* switch */

		  if ( scc == (lc = cc))
		    k++;
		}
		return(0);

	case '?':
	caseq:
		if(scc) return(amatch(++s, ++p));
		return(0);
	case '*':
		return(umatch(s, ++p));
	case 0:
		return(!scc);
	case '\\':
		c = *(++p);
    }
    if (c==scc)
      goto caseq;

    return(0);
}

int
umatch(const char * s, const char * p)

{
  if(*p==0)
    return(1);

  while(*s)
    if (amatch(s++, p))
      return(1);

  return(0);
}
