static char sccsid[] = "@(#)38	1.1  src/bos/usr/ccs/lib/libc/AFgetatr.c, libcgen, bos411, 9428A410j 12/14/89 17:37:02";
/*
 * LIBIN: AFgetatr
 *
 * ORIGIN: ISC
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
 * FUNCTION: Get the attribute value matching name.
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to the value of name.
 */

#include <stdio.h>
#include <string.h>
#include <IN/standard.h>
#include <IN/AFdefs.h>

char *
AFgetatr(ATTR_t at, char *name)
{
	for (;;)
	{   if (at->AT_name == NULL)
		return(NULL);
	    if (strcmp(at->AT_name,name) == 0)
		return(at->AT_value);
	    at++;
	}
}
