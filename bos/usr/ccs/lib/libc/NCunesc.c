static char sccsid[] = "@(#)91	1.3  src/bos/usr/ccs/lib/libc/NCunesc.c, libcnls, bos411, 9428A410j 6/11/91 09:46:06";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCunesc
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <NLctype.h>
#include <stdlib.h>

static int
__NCunesc(char * c, NLchar *nlc)
{
	return(NCunesc(c, nlc));
}

#ifdef NCunesc
#undef NCunesc
#endif

int
NCunesc(char * c, NLchar *nlc)
{
/*  Translate char escape string at c to single NLchar at nlc.
 */

    register int length;

    if (MB_CUR_MAX == 1) {
	if (c[0] != '\\' || c[1] != '<' || 
	    _NLunescval(&c[2],c[3] == '>' ? 1 : 2, nlc) < 0) { 
	    nlc[0] = c[0];
	}
	else {
	    if (c[3] == '>') return (4);
	    if (c[4] == '>') return (5);
	    nlc[0] = c[0];
	}
 	return(1);
    }
    else {
	/* return the character if not the start of an escape sequence
	 */
	if (c[0] != '\\' || c[1] != '<')  {
		nlc[0] = c[0];	
		return (1);
	}
	
	if (c[4] == '>')
		length = 2;
	else if (c[5] == '>')
		length = 3;
	else if (c[6] == '>')
		length = 4;
	else  {
		nlc[0] = c[0];	
		return (1);
	}

	/* start of a mneumonic in unesctab 
	 */
	if (c[2] == 'j')  {
		if (_NLunescval (&c[2], length, nlc) == -1)   {
			nlc[0] = c[0];
			return (1);
		}
		else
			return (length + 3);
	}

	if (ishexesc(&c[2]) == -1)  {
		nlc[0] = c[0];
		return (1);
	}
	else  {
		NCuneschex(c, nlc);
		return (length + 3);
	}
    }
}
