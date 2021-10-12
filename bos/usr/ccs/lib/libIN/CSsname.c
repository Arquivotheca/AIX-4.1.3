static char sccsid[] = "@(#)88	1.6  src/bos/usr/ccs/lib/libIN/CSsname.c, libIN, bos411, 9428A410j 6/10/91 10:15:13";
/*
 * LIBIN: CSname
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
 * FUNCTION: Return "simple" part of pathname.
 *
 * RETURN VALUE DESCRIPTION: 
 */

#include <IN/standard.h>

char *
CSsname (name)
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
	    if( *--cp == '/'
			     )
	    {   ++cp;
		if( *cp == NUL )
		    return ".";
		break;
	    }
        return cp;
}
