static char sccsid[] = "@(#)50	1.6  src/bos/usr/ccs/lib/libIN/CSdname.c, libIN, bos411, 9428A410j 6/10/91 10:14:47";
/*
 * LIBIN: CSdname
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
 * FUNCTION: Return "directory" part of pathname.
 *
 * NOTES:    This routine may modify its argument.
 *
 * RETURN VALUE DESCRIPTION: 
 */

#include <IN/standard.h>

char *
CSdname (name)
register char *name;
{
	register char *cp = name + CSlen(name);

        /* ignore trailing slashes */

	while( --cp > name && *cp == '/'
					 )
	    ;
        ++cp;

        /* find last slash */

	while( cp > name )
	{   if( *--cp == '/' )
	    {   if( cp == name )
		    ++cp;
		*cp = NUL;
		return name;
	    }
	}
	return ".";
}
