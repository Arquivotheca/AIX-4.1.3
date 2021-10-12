static char sccsid[] = "@(#)37	1.1  src/bos/usr/ccs/lib/libc/AFfndrec.c, libcgen, bos411, 9428A410j 12/14/89 17:36:09";
/*
 * LIBIN: AFfndrec
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
 * FUNCTION: Find the first record in an Attribute File which matches
 *	     the attributes in pattern.
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to an Attribute structure.
 */

#include <stdio.h>
#include <string.h>
#include <IN/standard.h>
#include <IN/AFdefs.h>

ATTR_t
AFfndrec(AFILE_t af, ATTR_t pattern)
{       ATTR_t cp, pp;
	char *str;

	while (AFnxtrec(af) != NULL)
	{   for (pp = pattern; pp->AT_name; pp++)
	    {   for (cp = af->AF_catr;
		  cp->AT_name && strcmp(pp->AT_name,cp->AT_name) != 0;
		  cp++) ;
		if (cp->AT_name == NULL)
		    break;
		if (pp->AT_value)
		{   str = cp->AT_value;
		    while (*str && (strcmp(pp->AT_value,str) != 0))
		    {   str = strchr(str,'\0');
			++str;
		    }
		    if (*str == 0)
			break;
		}
	    }
	    if (pp->AT_name == NULL)         /* if matched all patterns ... */
		return(af->AF_catr);      /* then current record is a match */
	}
	return(NULL);
}
