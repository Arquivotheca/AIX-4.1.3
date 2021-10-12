static char sccsid[] = "@(#)65	1.6  src/bos/usr/ccs/lib/libIN/CSgets.c, libIN, bos411, 9428A410j 6/10/91 10:52:19";
/*
 * LIBIN: CSgets
 *
 * ORIGIN: 9,10
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: 
 * CSgets reads chars from stream into dst until a newline, formfeed, or EOF is
 * encountered, or until cnt - 1 chars have been read.  A terminating newline or
 * formfeed is discarded, and a NULL is stored.
 *
 * RETURN VALUE DESCRIPTION: 
 * NULL is returned if EOF is encountered before any chars have been read.
 * Otherwise, the address of the NULL terminating dst is returned.  Lines longer
 * than cnt - 1 are only partially read.
 */

#include <stdio.h>
#include <IN/standard.h>

char *
CSgets(s, n, iop)
char *s;		/* buffer to contain line of characters */
int n;			/* max # bytes to get (including NULL) */
register FILE *iop;	/* stream pointer */
{
	register char *cs;
	register wint_t c;
	char mbstr[MB_LEN_MAX];
	char *pmb;
	int mbcount;

	cs = s;
	n--;        /* save one space for terminating NULL */
	for (;;)
	{   if ((c = fgetwc(iop)) == WEOF)
	    {   if (s == cs)
		    return(0);
		break;
	    }
	    mbcount = wctomb(mbstr, (wchar_t)c);
	    if (mbstr[0] == '\n')
		break;
	    if (mbstr[0] == '\f')
		break;
	    if (n < mbcount)
	    {	ungetwc(c, iop);
		break;
	    }
	    else
	    {   
		for (pmb = mbstr; mbcount > 0; mbcount--)
		{   *cs++ = *pmb++;
		    n--;
		}
	    }
	}
	*cs = '\0';
	return(cs);
}
