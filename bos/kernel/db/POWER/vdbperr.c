static char sccsid[] = "@(#)81	1.4  src/bos/kernel/db/POWER/vdbperr.c, sysdb, bos411, 9428A410j 6/16/90 03:05:51";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: vdbperr, abend_message
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/types.h>
#define  DEFINE_ERROR_TABLE
#include "vdberr.h"			/* Define the command error tab	*/
#undef   DEFINE_ERROR_TABLE

/*                                                                   
 * EXTERNAL PROCEDURES CALLED: 
 */


/*
 * NAME: vdbperr
 *                                                                    
 * FUNCTION:
 *   print the error passed in.
 *   Messages are concatinated if more than one is passed.
 *
 * NOTES:
 *   This routine uses a variable number of parameters.
 *   The first, and maybe only, parameter is an index into
 *   error_text.
 *   if subtext is true for that message, the next two are a
 *   string and another index.
 */

void
vdbperr(i,sub,j)
int i,j;
char *sub;
{

	/*
	 * Get the first parameter.
	 * if it has substitution text, print it and the 
	 * follow-on text.
	 */
	if (i >= MAXERR) {
		vdbperr(deb_err1,"vdbperr - undefined error number",deb_err2);
		return;
	}
	printf("%s",ERROR_PREFIX);
	printf("%s",error_text[i].msg);
	if (error_text[i].subtext)
		/* Put substitution text in quotes. */
		printf(" \"%s\" %s",sub,error_text[j].msg);
	printf("\n");
}


/*
 * NAME: abend_message
 *                                                                    
 * FUNCTION:
 *   Print the abend message with the given code.
 *
 */
void
abend_message(code)
ushort code;
{
	printf("%s%s\n",ABEND_PREFIX,abend_text[code-1]);
}
