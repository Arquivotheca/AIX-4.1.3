static char sccsid[] = "@(#)77	1.2  src/bos/diag/da/iop/get_mode.c, daiop, bos411, 9428A410j 12/17/92 11:27:39";
/*
 *   COMPONENT_NAME: DAIOP
 *
 *   FUNCTIONS: getdamode
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include "iop.h"
#include "diag/tm_input.h"
#include "diag/tmdefs.h"

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
	    return(mode);

}



