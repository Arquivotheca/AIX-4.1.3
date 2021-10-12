static char sccsid[] = "@(#)90	1.4  src/bos/usr/ccs/lib/libc/NCesc.c, libcnls, bos411, 9428A410j 6/11/91 09:45:56";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCesc
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
#include <stdlib.h> /** MB_CUR_MAX **/

static int 
__NCesc(NLchar *nlc, char *c)
{
	return(NCesc(nlc, c));
}

#ifdef NCesc
#undef NCesc
#endif

int NCesc(NLchar *nlc, char *c) 
{
    int retcode = 0;
    register int length;
    int rc;


    /**********
      for single byte codesets
    **********/
    if (MB_CUR_MAX == 1) {
	/* if NLchar value is less than 0x80 then return ascii character */
	if (*nlc < 0x80) { 
	    c[0] = *nlc;
	    retcode++;
	}
	else {
	    c[0] = '\\';
	    c[1] = '<';
	    if (_NLesctsize <= (unsigned)*(nlc)-0x80)  {
	        c[2] = '?';
	        c[3] = '?';
		retcode = 4;
	    }
	    else {
	        c[2] = _NLesctab[*(nlc)-0x80][0];
		c[3] = _NLesctab[*(nlc)-0x80][1];
		if (c[3]) {
		    c[4] = '>';
		    retcode = 5;
		}
		else {
		    c[3] = '>';
		    retcode = 4;
		}
	    }
	}
	return(retcode);
    }
    /**********
      932
    **********/
    else if (MB_CUR_MAX == 2) {
	if ((unsigned) *nlc < 0x80)   {
		c[0] = nlc[0];
		return (1);
	}

	/*
	  Try to find the escape sequence in NLesctab_932
	*/
	if (*nlc >= MINESCVAL && *nlc <= MAXESCVAL)
		return (_NLescval (nlc, c));

	/*
	  return a hex escape string
	*/
	NCeschex (nlc, c);
	return (7);
    }
}
