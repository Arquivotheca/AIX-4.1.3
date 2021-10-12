static char sccsid[] = "@(#)44  1.1  src/bldenv/sbtools/libsb/match.c, bldprocess, bos412, GOLDA411a 4/29/93 12:21:54";
/*
 * Copyright (c) 1990, 1991, 1992  
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
 * ODE 2.1.1
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

#  include <sys/file.h>

    char   * index ();

gmatch (s, p)

	/*
	 * The following match routine was taken from the "find" command.
	 * The "!" option was added to support "not".  The "\" was
	 * added to support handling the special characters literally.
	 * Returns true if string 's' is matched by pattern 'p' where 'p'
	 * is a string with wildcards in it.
	 */

    register char *s, *p;

{
  if ( *p != '!' )
	return amatch(s, p);
  else
	return ( ! amatch(s, ++p));
}

amatch(s, p)
    register char *s, *p;
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
		endbrace = index(p, '}');
		if (endbrace == (char *) 0) return 0;	/* No closing '}' */
		schar = *(scopy = s);

		while (cc = *++p) {
			if (cc == ',' || cc == '}') {
				k |= amatch(scopy, endbrace + 1);
				if (k) return 1;
				if (cc == '}') return 0;
				schar = *(scopy = s);	/* Retry source */
			}
			else
			if (cc == schar) {
				/* Char match succeeded */
				schar = *++scopy;
			} else {
				/* Char match failed */
				endcomma = index(p, ',');
				if (endcomma == (char *) 0 ||
				    endcomma >= endbrace)
					return 0;

				p = endcomma;	/* Next choice */
				schar = *(scopy = s);	/* Retry source */
			}
		}
		return 0;

	case '[':
		k = 0;

		while (cc = *++p) {
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

umatch(s, p)

    register char *s, *p;

{
  if(*p==0)
    return(1);

  while(*s)
    if (amatch(s++, p))
      return(1);

  return(0);
}
