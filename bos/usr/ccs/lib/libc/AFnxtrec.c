static char sccsid[] = "@(#)40	1.1  src/bos/usr/ccs/lib/libc/AFnxtrec.c, libcgen, bos411, 9428A410j 12/14/89 17:39:21";
/*
 * LIBIN: AFnxtrec
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
 * FUNCTION: Get the next record from an Attribute File. 
 *	     Merge in any default record that preceeds it.
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to an Attribute structure.
 */

#include <stdio.h>
#include <string.h>
#include <IN/standard.h>
#include <IN/AFdefs.h>

static char DEFAULT[] = "default";

ATTR_t
AFnxtrec(AFILE_t af)
{       register ATTR_t cp, pp;
	extern char *AFread();

	while (AFread(af))
	{   if (strcmp(af->AF_catr->AT_value,DEFAULT) == 0)   /* if default  */
	    {   register char *bp;         /* then swap current and default */
		bp = af->AF_cbuf;  af->AF_cbuf = af->AF_dbuf;  af->AF_dbuf = bp;
		cp = af->AF_catr;  af->AF_catr = af->AF_datr;  af->AF_datr = cp;
	    } else                                 /* else normal record... */
	    {   if (af->AF_datr->AT_value != NULL)  /* if default exists... */
		{   for (pp = af->AF_datr+1; pp->AT_name; pp++)/* ... merge */
		    {   for (cp = af->AF_catr+1; ; cp++)    /* into current */
			{   if (cp->AT_name == NULL) /* end of attr list... */
			    {   cp->AT_name = pp->AT_name;   /* append this */
				cp->AT_value = pp->AT_value;/* default attr */
				cp++;
				cp->AT_name = NULL;       /* terminate list */
				break;
			    }
			    if (strcmp(pp->AT_name,cp->AT_name) == 0)
				break;         /* this attribute overridden */
			}
		    }
		}
		return(af->AF_catr);
	    }
	}
	return(NULL);
}
