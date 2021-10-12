static char sccsid[] = "@(#)22	1.7  src/bos/usr/bin/gencat/catio.c, cmdmsg, bos411, 9428A410j 11/22/93 17:11:18";
/*
 * COMPONENT_NAME:  (CMDMSG) Message Catalogue Facilities
 *
 * FUNCTIONS: descopen, descgets, descclose, descset, descerrck
 *
 * ORIGINS: 27, 18
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */


#include <stdio.h>
FILE *descfil;

/*
 * EXTERNAL PROCEDURES CALLED: None
 */


/*
 * NAME: descopen
 *
 * FUNCTION: Perform message utility input operations
 *
 * EXECUTION ENVIRONMENT:
 *	User Mode.
 *
 * NOTES: The routines in this file are used to provide a mechanism by which
 *     	  the routines in gencat can get the next input character without
 *  	  worrying about details such as if the input is coming from a file
 *  	  or standard input, etc.
 *
 * RETURNS : 0 - successful open
 *          -1 - open failure
 */


/*   Open a catalog descriptor file and save file descriptor          */ 

#ifdef _NO_PROTO
descopen(filnam)
char *filnam;
#else
descopen(char *filnam) 
#endif /* _NO_PROTO */
			/*
			  filnam - pointer to message catalog name
			*/
{
	if ((descfil = fopen(filnam,"r")) == 0) 
		return (-1);
	
return (0);
}



/*
 * NAME: descgets
 *
 * FUNCTION: Read message catalog
 *
 * EXECUTION ENVIRONMENT:
 *	User mode.
 *
 * NOTES: Read the next line from the opened message catalog descriptor file.
 *
 * RETURNS: Pointer to message buffer -- scccessful
 *          NULL pointer -- error or end-of-file
 */


#ifdef _NO_PROTO
char *descgets(buff, bufsize)
char *buff; int bufsize;
#else
char *descgets(char *buff, int bufsize)
#endif /* _NO_PROTO */
			/*
			  buff - pointer to message buffer
			  bufsize - size of message buffer in bytes
			*/
{
	char *str;

	str = fgets(buff, bufsize, descfil);
	buff[bufsize-1] = '\0';        /* terminate in case length exceeded */
	return (str);
}



/*
 * NAME: descclose
 *
 * FUNCTION: Close message catalog
 *
 * EXECUTION ENVIRONMENT:
 *	User mode.
 *
 * NOTES: Close the message catalog descriptor file.
 *
 * RETURNS: None
 */


void
descclose()
{
	fclose(descfil);
	descfil = 0;
}



/*
 * NAME: descset
 *
 * FUNCTION: Establish message catalog file descriptor
 *
 * EXECUTION ENVIRONMENT:
 *	User mode.
 *
 * NOTES: Set the file descriptor to be used for message catalog access.
 *        
 *
 * RETURNS: None
 */


void
#ifdef _NO_PROTO
descset(infil)
FILE *infil;
#else
descset(FILE *infil)
#endif /* _NO_PROTO */
{
	descfil = infil;
}


/*
 * NAME: descerrck
 *
 * FUNCTION: Check I/O stream status
 *
 * EXECUTION ENVIRONMENT:
 *	User mode.
 *
 * RETURNS: 0 - no error encountered
 *         -1 - error encountered
 */

descerrck()
{
	if (ferror(descfil))
		return(-1);
	else    return(0);
}

