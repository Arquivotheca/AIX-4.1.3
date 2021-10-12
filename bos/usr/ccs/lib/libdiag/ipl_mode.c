static char sccsid[] = "@(#)46	1.4  src/bos/usr/ccs/lib/libdiag/ipl_mode.c, libdiag, bos411, 9430C411a 7/28/94 15:44:41";
/*
 * COMPONENT_NAME: (LIBDIAG) Diagnostic Library
 *
 * FUNCTIONS: ipl_mode
 *            diag_exec_source
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include	"diag/diag.h"
#include	"diag/tmdefs.h"

/*
 * NAME: ipl_mode  
 *
 * FUNCTION: Determine if running in service IPL mode, and whether diagnostics
 * 	     is running off cdrom or tape.
 * 
 * NOTES: This functions checks environment variables to determine the
 *	  execution mode.
 *
 * RETURNS:
 *	1 - EXENV_IPL  - Diagnostics invoked during IPL
 *	2 - EXENV_STD  - Standalone Diagnostics
 *	3 - EXENV_REGR - Regression mode Diagnostics
 *	4 - EXENV_CONC - Concurrent Diagnostics
 *	5 - EXENV_SYSX - System Exerciser Diagnostics
 *
 * 	The int argument 'stand_alone' variable will be set to the following values
 *		0 - DIAG_FALSE if DIAG_IPL_SOURCE is = NULL
 *		          			     = IPL_SOURCE_DISK
 *		                		     = IPL_SOURCE_LAN
 *		1 - DIAG_TRUE if DIAG_IPL_SOURCE is  = IPL_SOURCE_CDROM
 *		         			     = IPL_SOURCE_TAPE
 */

ipl_mode( stand_alone )
int	*stand_alone;
{

	int	exenv_flag;
	char	*environment;

	/* Environment variable DIAG_ENVIRONMENT is set to:
         *	EXENV_IPL  - Diagnostics invoked during IPL
	 *	EXENV_STD  - Standalone Diagnostics
	 *	EXENV_REGR - Regression mode Diagnostics
	 *	EXENV_CONC - Concurrent Diagnostics
	 *	EXENV_SYSX - System Exerciser Diagnostics
	 */
	if (( environment = (char *)getenv("DIAG_ENVIRONMENT")) == NULL )
		exenv_flag = EXENV_STD;
	else
		sscanf( environment, "%d", &exenv_flag );


	/* Environment variable DIAG_IPL_SOURCE should have at this point 
           one of the following values:
		NULL (not set)
		IPLSOURCE_DISK                  0
		IPLSOURCE_CDROM                 1
		IPLSOURCE_TAPE                  2
		IPLSOURCE_LAN                   3
	 */
	if (( environment = (char *)getenv("DIAG_IPL_SOURCE")) == NULL )
		*stand_alone = DIAG_FALSE;
	else
		switch ( *environment ) {
			case '1':
			case '2':
				*stand_alone = DIAG_TRUE;
				break;
			case '0':
			case '3':
			default:
				*stand_alone = DIAG_FALSE;
		}

	return(exenv_flag);
}

/*
 * NAME: diag_exec_source
 *
 * FUNCTION: Determine where the diagnostics program are run from.
 * 	     If not from hard file, then mount_point will return
 *	     the directory where the file system reside(CDRFS).
 * 
 * NOTES: This functions checks environment variables to determine the
 *	  source of program execution.
 *
 * RETURNS:
 *	0 - Running off hard file
 *	1 - Running from cdrom
 *
 */

diag_exec_source(mount_point)
char	*mount_point;
{
	char	*diagenv;
	int	diag_source;

        if((diagenv = (char *)getenv("DIAG_SOURCE")) == (char *)NULL)
			diag_source=0;
	else
			diag_source=atoi(diagenv);
	if(diag_source){
		diagenv=(char *)getenv("CDRFS_DIR");
		strcpy(mount_point, diagenv);
	}
	return(diag_source);
}
