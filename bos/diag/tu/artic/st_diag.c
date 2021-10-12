static char sccsid[] = "@(#)29  1.2.1.2  src/bos/diag/tu/artic/st_diag.c, tu_artic, bos411, 9428A410j 8/19/93 17:52:27";
/* COMPONENT_NAME:  
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * FUNCTIONS: start_diag_tu
 *
 */
#include <stdio.h>
#include <artictst.h>
/*
 * NAME: start_diag_tu
 *
 * FUNCTION: Start all the diagnostic test units
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will start all the diagnostic test units running
 *          on the adapter card
 *
 * RETURNS: A zero (0) for successful completition or non-zero for error.
 *
 */
int start_diag_tu (fdes, tucb_ptr, command_type, retc)
   int fdes;
   TUTYPE *tucb_ptr;
   int command_type;  /* Command */
   int retc; /* Return Code to return if an error was found */
   {
        char *s;      /* pointer to string to print */
        unsigned char ec;
        int rc;
        extern int start_task();

        ec = 0;
        if (((rc=start_task(fdes,tucb_ptr,command_type,&ec)) != 0) ||
                                (ec != 0) )
           {
                if (rc!=0)
                  return(rc);
                else
                  return(retc); /* else return error code sent in */
           }

        return(0);
   }
