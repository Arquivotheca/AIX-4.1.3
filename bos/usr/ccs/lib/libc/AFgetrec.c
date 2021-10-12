static char sccsid[] = "@(#)39	1.1  src/bos/usr/ccs/lib/libc/AFgetrec.c, libcgen, bos411, 9428A410j 12/14/89 17:38:22";
/*
 * LIBIN: AFgetrec
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
 * FUNCTION: Find the first record in an Attribute File which has a
 *	     given name.
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to an Attribute structure.
 */

#include <stdio.h>
#include <string.h>
#include <IN/standard.h>
#include <IN/AFdefs.h>

ATTR_t
AFgetrec(AFILE_t af, char *name)
{       register ATTR_t cp;
	register char *p;

	while (AFnxtrec(af))
	{   cp = af->AF_catr;
	    if ((p = cp->AT_value) != NULL)
	    {   while (*p)
		{   if (strcmp(p,name) == 0)
			return(af->AF_catr);   /* then current record is it */
		    p = strchr(p,'\0') + 1;
		}
	    }
	}
	return(NULL);
}
