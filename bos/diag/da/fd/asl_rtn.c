static char sccsid[] = "@(#)69  1.4  src/bos/diag/da/fd/asl_rtn.c, dafd, bos411, 9428A410j 12/17/92 10:57:14";
/*
 *   COMPONENT_NAME: dafd
 *
 *   FUNCTIONS: chk_asl_return
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



#include "diag/diag_exit.h"
#include "diag/diago.h" 
#include "diag/tmdefs.h" 
#include "diag/tm_input.h" 

/*
 * NAME: void chk_asl_return(asl_return)
 *                                                                    
 * FUNCTION: Evaluate the return code from the diag_asl_display() 
 *           functions. If the user has struck the 'cancel' or 'exit' keys
 *           the process exits back to the diagnostic controller.  
 *           If a hardware mismatch or error is discovered the 
 *           process exits back to the controller with 
 *           DA_ERROR_OTHER. 
 *
 * EXECUTION ENVIRONMENT: called from all menu & message display functions
 *                        SEE: dfd_menus.c & dfda_menus.c
 *                                                                   
 *
 * RETURNS: NONE
*/

void chk_asl_return(asl_return)
long asl_return;
{
	extern struct tm_input da_input;
	extern void clean_up();

	switch(asl_return)
	{
	case DIAG_ASL_OK:
		break;
	case DIAG_ASL_ERR_SCREEN_SIZE:
	case DIAG_ASL_FAIL:
	case DIAG_ASL_ERR_NO_TERM:
	case DIAG_ASL_ERR_NO_SUCH_TERM:
	case DIAG_ASL_ERR_INITSCR:
		DA_SETRC_ERROR(DA_ERROR_OTHER);
		if (da_input.exenv == EXENV_SYSX) {
			if (da_input.console == CONSOLE_TRUE)
				diag_asl_quit();
		}
		clean_up();
		break; 
	case DIAG_ASL_EXIT:
		DA_SETRC_USER(DA_USER_EXIT);
		if (da_input.exenv == EXENV_SYSX) {
			if (da_input.console == CONSOLE_TRUE)
				diag_asl_quit();
		}
		clean_up();
		break;
	case DIAG_ASL_CANCEL:
		DA_SETRC_USER(DA_USER_QUIT);
		if (da_input.exenv == EXENV_SYSX) {
			if (da_input.console == CONSOLE_TRUE)
				diag_asl_quit();
		}
		clean_up();
		break; 
	default:
		break;

	}

	return;
}



