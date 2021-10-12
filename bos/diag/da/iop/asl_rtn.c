static char sccsid[] = "@(#)71	1.2  src/bos/diag/da/iop/asl_rtn.c, daiop, bos411, 9428A410j 12/17/92 11:26:25";
/*
 *   COMPONENT_NAME: DAIOP
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

extern void clean_up();

/*
 * NAME:  void chk_asl_return(asl_return)
 *                                                                    
 * FUNCTION: Check the value returned from the diag_asl_display() functions
 *       
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *                                                                   
 * RETURNS: NONE
*/

void chk_asl_return(asl_return)
long asl_return;
{
	switch(asl_return)
	{
	case DIAG_ASL_OK:
		break;
	case DIAG_ASL_ERR_SCREEN_SIZE:
	case DIAG_ASL_FAIL:
	case DIAG_ASL_ERR_NO_TERM:
	case DIAG_ASL_ERR_NO_SUCH_TERM:
	case DIAG_ASL_ERR_INITSCR:
		{
			DA_SETRC_ERROR(DA_ERROR_OTHER);
			clean_up();
			break;
		}
	case DIAG_ASL_EXIT:
		{
			DA_SETRC_USER(DA_USER_EXIT);
			clean_up();
			break;
		}
	case DIAG_ASL_CANCEL:
		{
			DA_SETRC_USER(DA_USER_QUIT);
			DA_SETRC_MORE(DA_MORE_NOCONT);
			clean_up();
			break;
		}
	default:
		break;

	}
}



