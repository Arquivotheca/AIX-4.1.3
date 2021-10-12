static char sccsid[] = "@(#)23  1.2.1.2  src/bos/diag/tu/artic/rcp_stat.c, tu_artic, bos411, 9428A410j 8/19/93 17:52:23";
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
 * FUNCTIONS: RCPStatDone
 *
 */

#include <stdio.h>
#include <artictst.h>

/*
 * NAME: RCPStatDone
 *
 * FUNCTION: Returns the status of the RCP done bit (Byte 0x47C).
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will cause a byte to be read from the adapter's
 *          RAM location (0x47C).  If this value is 0x03, then the diagnostic
 *          code has been loaded and is running.  If and other value is found
 *          the diagnostic code is not performing properly.
 *
 * RETURNS: A zero (0) for RCP done or one (1) for RCP not done.
 *
 */
int RCPStatDone (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int rc;
        unsigned char value;

        extern int outby();
        extern int dh_diag_mem_read();

        /* Setup CPU page register to be 0 */
        outby(fdes,tucb_ptr, adapter_CPUPG,0x00,GATE_RAM1);

        if ((rc = dh_diag_mem_read(fdes,tucb_ptr,
                                0,(unsigned short) 0x47C,&value)) == 0)
           {
                if (value == 3)
                        return(0x00);
           }

        return(1);
   }
