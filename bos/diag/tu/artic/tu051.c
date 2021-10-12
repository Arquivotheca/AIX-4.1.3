static char sccsid[] = "@(#)78  1.2.1.2  src/bos/diag/tu/artic/tu051.c, tu_artic, bos411, 9428A410j 8/19/93 17:48:11";
/* COMPONENT_NAME:  
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * FUNCTIONS: tu051
 *
 */

#include <stdio.h>
#include <artictst.h>

/*
 * NAME: tu051
 *
 * FUNCTION: CIO Test Unit
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will initiate the CIO test unit.  This is more
 *          thorough test than the POST test.  This test will check the bit
 *          port pattern recognition, interrupts, reset, timer 1 and timer 2.
 *          There is a prerequisite that TU019 be executed before executing
 *          this test unit.
 *
 * RETURNS: The return code from the CIO test.
 *
 */
int tu051 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        extern int start_diag_tu();

        return(start_diag_tu(fdes, tucb_ptr, CIO_COM_PCODE, CIO_ER));
   }
