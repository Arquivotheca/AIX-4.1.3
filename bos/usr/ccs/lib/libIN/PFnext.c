static char sccsid[] = "@(#)39	1.6  src/bos/usr/ccs/lib/libIN/PFnext.c, libIN, bos411, 9428A410j 6/10/91 10:22:11";
/*
 * LIBIN: PFnext
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
 * FUNCTION: Get next password file entry.
 *      (If the file hasn't been opened, get the first entry in the file.)
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to pwinfo structure.
 */

#include <stdio.h>
#include <IN/standard.h>
#include <IN/PFdefs.h>

char *CSlocc(), *CSgetl();

struct pwinfo *
PFnext(i)
register struct pwinfo *i;
{       register char *p;
	register char **u;
	register int j;

	if (i->_ufile == NULL)
	    i->_ufile = fopen(i->_ufilename,"r");
	if (i->_ufile != NULL)
	{   if (CSgetl(i->_ubuf,i->_usbuf,i->_ufile) != NULL)
	    {   p = i->_ubuf;
		u = i->_uargs;
		if ((j = i->_unargs) > 0)
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
	i->_uargs[0] = 0;       /* couldn't open file, or EOF */
	return(0);
}
