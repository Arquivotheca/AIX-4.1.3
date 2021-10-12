static char sccsid[] = "@(#)30	1.6  src/bos/usr/ccs/lib/libIN/PFname.c, libIN, bos411, 9428A410j 6/10/91 10:22:06";
/*
 * LIBIN: PFname
 *
 * ORIGIN: 9
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
 * FUNCTION: Get password file entry for username s.
 *      (If s == NULL, gets the first entry in the file.)
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to pwinfo structure.
 */

#include <stdio.h>
#include <IN/standard.h>
#include <IN/PFdefs.h>

char *CSlocc(), *CSgetl();

struct pwinfo *
PFname(i,s)
register struct pwinfo *i;
char *s;
{       register char *p;
	register char **u;
	register int j;

	if (i->_ufile == NULL)
	    i->_ufile = fopen(i->_ufilename,"r");
	if (i->_ufile != NULL)
	{   fseek(i->_ufile,0L,0);
	    while (CSgetl(i->_ubuf,i->_usbuf,i->_ufile) != NULL)
	    {   p = i->_ubuf;
		p = CSlocc(p,':');
		if (*p)
		    *p++ = '\0';
		if (s == NULL || CScmp(i->_ubuf,s) == 0)
		{   u = i->_uargs;
		    *u++ = i->_ubuf;
		    if ((j = i->_unargs - 1) > 0)
		    {   do
			{   *u++ = p;
			    p = CSlocc(p,':');
			    if (*p)
				*p++ = '\0';
			} while (--j);
		    }
		    return(i);
		}
	    }
	}
	i->_uargs[0] = 0;       /* couldn't find username */
	return(0);
}
