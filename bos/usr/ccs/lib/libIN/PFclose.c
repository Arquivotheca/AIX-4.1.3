static char sccsid[] = "@(#)74	1.6  src/bos/usr/ccs/lib/libIN/PFclose.c, libIN, bos411, 9428A410j 6/10/91 10:21:39";
/*
 * LIBIN: PFclose
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
 * FUNCTION: Close the password file.
 *
 * RETURN VALUE DESCRIPTION: void
 */

#include <stdio.h>
#include <IN/standard.h>
#include <IN/PFdefs.h>

PFclose(i)
register struct pwinfo *i;
{
	fclose(i->_ufile);
	i->_ufile = NULL;
}
