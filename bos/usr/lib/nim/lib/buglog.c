/* @(#)98	1.1  src/bos/usr/lib/nim/lib/buglog.c, cmdnim, bos411, 9428A410j  6/17/93  12:26:38 */
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: bugopen, buglog
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h> 
#include <varargs.h> 

/* 
 *	DEFINES  
 */ 

#define	NIMBUG		"NIMBUG"
#define	NIMBUG_DEF	"/dev/console"

/* 
 *	GLOBALS 
 */ 
	int	debug = 0;
	FILE  	*bugFP;
	char	*bugFile;
	char	bugTerm[4];

/*----------------------------- buglog 
 *
 * NAME: buglog
 *
 * FUNCTION:   Checks the debug flag, if set uses the variable input 
 *    parameters to write a line of text to a file pointer. 
 *
 *    Call this chap like this: 
 *    buglog("%s: a format string", argv[0] );
 *
 *
 * DATA STRUCTURES:
 *    parameters:
 *    global:
 *       debug : checks to see if set. 
 *
 * RETURNS:
 *    0 - Not in debug mode.
 *    1 - everything is ok (probably)
 *
 *----------------------------------------------------------------------------*/


buglog(va_alist)
va_dcl
{
	va_list  ptr;

	char	*fmt;

	if (!debug)
		return(0);

	va_start(ptr);
	fmt = va_arg(ptr, char * );

	vfprintf(bugFP, fmt, ptr);
	fprintf(bugFP, bugTerm);
	fflush(bugFP);
	return(1);
}

/*----------------------------- bugopen 
 *
 * NAME: bugopen
 *
 * FUNCTION:   Opens a debug file.
 * 
 *
 * DATA STRUCTURES:
 *    parameters:
 *	NONE
 *    global:
 *       debug : checks to see if set. 
 *
 * RETURNS:
 *    0 - Not in debug mode.
 *    1 - everything is ok (probably)
 *
 *----------------------------------------------------------------------------*/
bugopen()
{

	/* 
 	 * First check if user said a special place to put the 
	 * debug information via the NIMBIG shell environment var. 
	 * Default to dev console if not.. 
	 */
	if ( (bugFile = getenv("NIMBUG")) == NULL ) 
		bugFile = "/dev/console";

	/* 
 	 * If the file is dev console terminate strings with \r\n
	 */
	if (strcmp(bugFile, "/dev/console") == 0 )
		strcpy(bugTerm, "\r\n");
	else
		strcpy(bugTerm, "\n");

	debug = ((bugFP = fopen(bugFile, "a")) != NULL);
}
