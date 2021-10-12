static char sccsid[] = "@(#)89  1.3  src/bos/diag/da/fd/get_mode.c, dafd, bos411, 9428A410j 12/17/92 10:59:25";
/*
 *   COMPONENT_NAME: dafd
 *
 *   FUNCTIONS: getdamode
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#define NEW_DIAG_CONTROLLER

#include <stdio.h>
#include "fd_set.h"
#include "diag/tm_input.h"
#include "diag/tmdefs.h"


/*
 * NAME: long getdamode( da_input)
 *                                                                    
 * FUNCTION: determine the test mode from the da_input structure passed. 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: INTERACTIVE_TEST_MODE ( 2 ) 
            NO_VIDEO_TEST_MODE    ( 1 )
            TITLES_ONLY_TEST_MODE ( 3 )
*/


long getdamode( da_input)
struct tm_input da_input;

{
	/* ........................................................... */
	/* the following variables were used to make the function more */
	/* readable and compact                                        */
	/* ........................................................... */

	short a = da_input.advanced;
	short c = da_input.console;
	short s = da_input.system;
	short l = da_input.loopmode;
	short e = da_input.exenv;

	int mode   = INVALID_TM_INPUT;

	if(c  == CONSOLE_TRUE  && ( s != SYSTEM_TRUE || l != LOOPMODE_INLM ) )
		mode = INTERACTIVE_TEST_MODE ;

	if( c == CONSOLE_FALSE )
		mode = NO_VIDEO_TEST_MODE;

	if( c == CONSOLE_TRUE && ( s == SYSTEM_TRUE ||  l == LOOPMODE_INLM))
	        mode = TITLES_ONLY_TEST_MODE ;

	if( e == EXENV_SYSX ) 
	{
		if (l == LOOPMODE_ENTERLM)
	        	mode = INTERACTIVE_TEST_MODE ;
		else
	        	mode = TITLES_ONLY_TEST_MODE ;
	}

	    return(mode);

}



