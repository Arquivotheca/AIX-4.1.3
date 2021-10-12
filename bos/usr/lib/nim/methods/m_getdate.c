/* @(#)65	1.4  src/bos/usr/lib/nim/methods/m_getdate.c, cmdnim, bos411, 9428A410j  8/7/93  14:17:53 */
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: get_date
 *		main
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



#include "cmdnim_mstr.h"

/* --------------------------- get_date  
 *
 * NAME: get_date
 *
 * FUNCTION:
 *	writes the current date/time to the specified file pointer in a format
 *	which can be used to set the date/time
 *	also writes the current TZ setting
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *	parameters:
 *		fp	= file pointer for writing to
 *		global:
 *
 * RETURNS: 
 *    SUCCESS:  Everything worked ok
 *    FAILURE:  Got a problem.
 *
 * OUTPUT:
 * ----------------------------------------------------------------------------*/
int
get_date(FILE * fp)
{
	int             rc = 0;
	char           *tz;
	time_t          timer;
	char            date[80];

	/*
	 * get the current TZ setting 
	 */
	if ((tz = getenv("TZ")) == NULL)
		tz = DEFAULT_TZ;

	/*
	 * get the current date & time 
	 */
	if ((timer = time(NULL)) == -1)
		rc = 1;
	else if (strftime(date, 80, "%m%d%\H%M.%\S%y", localtime(&timer)) > 0)
		fprintf(fp, "%s %s\n", date, tz);
	else
		rc = 1;

	return (rc);
}	


/*
 * --------------------------- Main m_getdate 
 *
 * NAME: Main 
 *
 * FUNCTION: 
 *
 * EXECUTION ENVIRONMENT: When this  method is called the stderr file pointer
 * is already open and a nimesis process is monitoring it for completion status
 * information.  The stdout file pointer is pointing to a socket opened (if
 * applicable) and being read by the initiating client. 
 *
 * RETURNS: 
 * 		SUCCESS:  Everything worked ok 
 * 		FAILURE:  Got a problem. 
 *
 * ---------------------------------------------------------------------- */

main(argc, argv)
	int             argc;
	char           *argv[];

{
	int             rc;

	/*
	 * Do NIM set up stuff. 
	 */
	if (nim_init(argv[0], ERR_POLICY_METHOD, NULL, NULL) != SUCCESS)
		exit(1);

	/* 
	 * Now call the function to get the date, time and time zone.
	 */
	rc = get_date(stdout);
	exit(rc);
}
