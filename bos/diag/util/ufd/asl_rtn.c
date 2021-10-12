static char sccsid[] = "@(#)13  1.2  src/bos/diag/util/ufd/asl_rtn.c, dsaufd, bos411, 9428A410j 5/2/91 10:21:51";
/*
 * COMPONENT_NAME: TUDKST
 *
 * FUNCTIONS: chk_asl_return();
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "diag/diag_exit.h"
#include "diag/diago.h"
#include "ufd_msg.h"

void chk_asl_return(asl_return)
long asl_return;
{
        extern void clean_up();
        extern void DFD_ERROR_message();

        switch(asl_return)
        {
        case DIAG_ASL_OK:
                break;
        case DIAG_ASL_ERR_SCREEN_SIZE:
        case DIAG_ASL_FAIL:
        case DIAG_ASL_ERR_NO_TERM:
        case DIAG_ASL_ERR_NO_SUCH_TERM:
        case DIAG_ASL_ERR_INITSCR:
                        DFD_ERROR_message(NOT_USABLE);
                        clean_up();
                        break;
        case DIAG_ASL_EXIT:
                        clean_up();
                        break;
        case DIAG_ASL_CANCEL:
                        clean_up();
                        break;
        default:
                break;

        }
}



