static char sccsid[] = "@(#)63	1.6  src/bos/usr/ccs/lib/libIN/PFageless.c, libIN, bos411, 9428A410j 6/10/91 10:21:35";
/*
 * LIBIN: PFageless
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
 * FUNCTION: Strip off password aging string in place.
 *
 * RETURN VALUE DESCRIPTION: void
 */

#include <IN/standard.h>
#include <IN/CSdefs.h>
extern  char *_uargs[];

PFageless()
{       if (_uargs[1] != NULL)
	    *CSlocc(_uargs[1], ',') = '\0';
}
