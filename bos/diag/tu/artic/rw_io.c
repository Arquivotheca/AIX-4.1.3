static char sccsid[] = "@(#)94  1.3  src/bos/diag/tu/artic/rw_io.c, tu_artic, bos411, 9428A410j 8/19/93 17:49:13";
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
 * FUNCTIONS: dh_diag_io_read, dh_diag_io_write
 *
 */
#include <stdio.h>
#include <artictst.h>

/*
 * NAME: DH_DIAG_IO_READ
 *
 * FUNCTION: Read one byte from the adapter register.
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will issue the Device Driver's register read IOCTL.
 *          via the C Library call icareadio.
 *
 * RETURNS: A zero (0) for no error or a non-zero for error condition
 *
 */

int dh_diag_io_read (fdes, tucb_ptr, reg, value)
   int fdes;
   TUTYPE *tucb_ptr;
   unsigned char reg;
   unsigned char *value;
   {
        unsigned short rc;

        rc = icareadio(fdes, reg, value);
        if (rc)
           {
                return((int)rc);
           }

        return(0);
   }

/*
 * NAME: DH_DIAG_IO_WRITE
 *
 * FUNCTION: Write one byte to the adapter register.
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will issue the Device Driver's register write IOCTL
 *          via the C Library call icawriteio.
 *
 * RETURNS: A zero (0) for no error or a non-zero for error condition
 *
 */
int dh_diag_io_write (fdes, tucb_ptr, reg,value)
   int fdes;
   TUTYPE *tucb_ptr;
   unsigned char reg;
   unsigned char value;
   {
        unsigned short rc;


        rc = icawriteio(fdes, reg, value);
        if (rc)
           {
                return((int)rc);
           }

        return(0);
   }
