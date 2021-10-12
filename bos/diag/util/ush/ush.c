static char sccsid[] = "@(#)45	1.1  src/bos/diag/util/ush/ush.c, cmddiag, bos411, 9428A410j 10/15/93 10:32:21";
/*
 * COMPONENT_NAME: (DUTIL) DIAGNOSTIC UTILITY
 *
 * FUNCTIONS:  None. 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifdef DEBUG
#define DBUGSTR(s)	printf(s);
#define DBUGPRMT  { printf("Type <Enter> to go back to the Menu:"); \
		  fflush(stdout); getchar(); }
#else
#define DBUGSTR(s)	
#define DBUGPRMT
#endif

#define CLS	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

#include <stdio.h>
#include <login.h>              /* S_PRIMARY ... */

/*
This is a service aid allowing the user to escape to shell, when the 
system is booted in service mode. This asks the user for the root 
passwd atmost three times and creates a bourne shell, if entered 
password is valid. 
*/
main() 
{
int logtimes=3;

	/* clear the screen and make user aware of the happenings */
	CLS;
	while (logtimes--)
		if (ckuserID("root", S_PRIMARY))
			continue;
		else
			break;

	if (logtimes < 0) /* trying to fool? chuck'im out! */
	{
		DBUGSTR("Failure\n");
		DBUGPRMT;
		exit(1);
	}

	/* execute /bin/bsh here */
	system("/bin/bsh");
}	
