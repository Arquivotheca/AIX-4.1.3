static char sccsid[] = "@(#)34  1.2.1.2  src/bos/diag/tu/artic/tu005.c, tu_artic, bos411, 9428A410j 8/19/93 17:52:44";
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
 * FUNCTIONS: tu005
 *
 */

#include <stdio.h>
#include <artictst.h>

/*
 * NAME: tu005
 *
 * FUNCTION: RAM Test Unit
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will initiate the RAM test unit.  This is a more
 *          thorough test than the POST test.  This test will check all the
 *          RAM bits except for the section of RAM containing the Diagnostic
 *          Microcode.  A five (5) pattern set is checked with each byte: 0,
 *          01, AA, 55 and FF.  There is a prerequisite that TU019 be executed
 *          before executing this test unit.
 *
 * RETURNS: The return code from the RAM test.
 *
 */
int tu005 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int rc;
        extern int start_diag_tu();

        rc = start_diag_tu(fdes, tucb_ptr, RAM_COM_CODE, LDRAM_ER);
        if ((rc >= 0x20) && (rc <= 0x23))
           {
                if ((rc == 0x20) || (rc == 0x21))
                        return(LDRAM_ER);
                return(HDRAM_ER);
           }
        return(rc);
   }
