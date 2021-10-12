static char sccsid[] = "@(#)33  1.2.1.2  src/bos/diag/tu/artic/tu004.c, tu_artic, bos411, 9428A410j 8/19/93 17:52:40";
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
 * FUNCTIONS: tu004
 *
 */

#include <stdio.h>
#include <artictst.h>

/*
 * NAME: tu004
 *
 * FUNCTION: CPU Test Unit  -- for Sandpiper V adapters.
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This function will initiate the CPU test unit.  This is the
 *          advance CPU test which is more thorough than the POST test.
 *          This test will check all the conditional flags and writes a
 *          pattern to all registers.  It tests the DMA allocation logic,
 *          interrupt controller and internal timers.  There is a prerequisite
 *          that TU019 be executed before attempting this test unit.
 *
 * RETURNS: The return code from the CPU test.
 *
 */
int tu004 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        extern int start_diag_tu();

        return(start_diag_tu(fdes, tucb_ptr, CPU_COM_CODE, CPU_ER));
   }
