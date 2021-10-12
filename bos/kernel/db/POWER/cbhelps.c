static char sccsid[] = "@(#)46	1.4  src/bos/kernel/db/POWER/cbhelps.c, sysdb, bos411, 9428A410j 6/16/90 03:00:06";

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: debpg
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

/*                                                                   
 * EXTERNAL PROCEDURES CALLED: 
 */

extern	char *	getterm();		/* Get from the terminal.	*/

/*
 * NAME: debpg
 *                                                                    
 * FUNCTION:
 *	. Provide a paging facility for control block formatters.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	Executes as part of the low-level debugger.
 *
 * RETURN VALUE DESCRIPTION:
 *	TRUE=Another page is desired.
 *	FALSE=Quit, x was entered.
 */  

char	PAGE_MSG[] = "Press \"ENTER\" to continue, or \"x\" to exit:";

int
debpg()
{
	printf(PAGE_MSG);
	return( (*getterm()=='x')? FALSE: TRUE );
}
