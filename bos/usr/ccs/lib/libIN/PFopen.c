static char sccsid[] = "@(#)50	1.6  src/bos/usr/ccs/lib/libIN/PFopen.c, libIN, bos411, 9428A410j 6/10/91 10:22:17";
/*
 * LIBIN: PFopen
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
 * FUNCTION: Open the password file.
 *
 * RETURN VALUE DESCRIPTION: Returns a FILE pointer to password file.
 */

#include <stdio.h>
#include <IN/standard.h>
#include <IN/PFdefs.h>

FILE *
PFopen(i,n)
register struct pwinfo *i;
register char *n;
{
	if (n == NULL)
	    n = i->_ufilename;
	return (i->_ufile = fopen(n,"r"));
}
