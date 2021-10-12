static char sccsid[] = "@(#)32  1.2.1.2  src/bos/diag/tu/artic/tu003.c, tu_artic, bos411, 9428A410j 8/19/93 17:52:34";
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
 * FUNCTIONS: interrupt_test, tu003
 *
 */

#include <stdio.h>
#include <artictst.h>
/*
 * NAME: interrupt_test
 *
 * FUNCTION: Test to see if the interrupts have occurred
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will check to see if the tasks are loaded and
 *          running, which means that the Load_Diagnostic and Start_Diagnostic
 *          interrupts had to haved occurred.
 *
 * RETURNS: A zero (0) for successful completition or non-zero for error.
 *
 */
int interrupt_test (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        extern int RCPStatDone();

        return(RCPStatDone(fdes, tucb_ptr));
   }

/*
 * NAME: tu003
 *
 * FUNCTION: Interrupt Test Unit -- for Sandpiper V adapters.
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This function will initiate the Interrupt test unit and
 *          return its return code.
 *
 * RETURNS: The return code from the Interrupt test.
 *
 */
int tu003 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        return(interrupt_test(fdes, tucb_ptr));
   }
