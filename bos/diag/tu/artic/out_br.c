static char sccsid[] = "@(#)22  1.2.1.2  src/bos/diag/tu/artic/out_br.c, tu_artic, bos411, 9428A410j 8/19/93 17:52:20";
/*
 * COMPONENT_NAME:  
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
 * FUNCTIONS: outby, outrm
 *
 */
#include <stdio.h>
#include <artictst.h>
typedef unsigned char byte;
/*
 * NAME: outby
 *
 * FUNCTION: Write a byte to the given register in the gate array.
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will cause the given byte to be written to
 *          given register.  It will check to see if the byte was written
 *          properly by performing a read byte and checking the value read
 *          with the value that was written.
 *
 * RETURNS: A zero (0) for successful completition or non-zero for error.
 *
 */
int outby (fdes, tucb_ptr, reg, value, retc)
   int fdes;
   TUTYPE *tucb_ptr;
   int reg;
   unsigned char value;
   short retc;
   {
        int rc;
        unsigned char check,orig;

        extern int dh_diag_io_write();
        extern int dh_diag_io_read();

        /* Read in the original value   */
        if ((rc=dh_diag_io_read(fdes, tucb_ptr,(byte) reg, &orig )) != 0)
                return(rc);
        /* Write the given value to the register        */
        if ((rc=dh_diag_io_write(fdes,tucb_ptr,(byte) reg,value)) != 0)
                return(REG_ERR);
        /* Read in the value just written       */
        if ((rc=dh_diag_io_read(fdes,tucb_ptr,(byte) reg,&check)) != 0)
                return(rc);
        /* Check the value read against the value written       */
        if (check != value)
        {
                /* Check the value read against the original value, if  */
                /* the original value is not equal to the value written */
                if ((check != orig) && (value != orig))
                {
                        return(REG_ERR);
                }

                return(retc);
        }

        return(0);
   }

/*
 * NAME: outrm
 *
 * FUNCTION: Write a byte to the adapter RAM.
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will cause the given byte to be written to
 *          given offset within card_page 0.  It will check to see if the byte
 *          was written properly by performing a read byte and checking the
 *          value read with the value that was written.
 *
 * RETURNS: A zero (0) for successful completition or non-zero for error.
 *
 */
int outrm (fdes, tucb_ptr, addr, value, retc)
   int fdes;
   TUTYPE *tucb_ptr;
   int addr;          /* Address to put value at */
   byte value;        /* Value to put in RAM     */
   short retc;
   {
        int rc;
        byte check,orig;

        extern int dh_diag_mem_write();
        extern int dh_diag_mem_read();

        /* Read in the original value   */
        if ((rc=dh_diag_mem_read(fdes,tucb_ptr,
                                0,(unsigned short)addr,&orig)) != 0)
                return(rc);
        /* Write the given value to memory      */
        if ((rc=dh_diag_mem_write(fdes,tucb_ptr,
                                0,(unsigned short) addr,
                                &value)) != 0)
                return(REG_ERR);
        /* Read in the value just written       */
        if ((rc=dh_diag_mem_read(fdes,tucb_ptr,
                                0,(unsigned short)addr,
                                &check)) != 0)
                return(rc);
        /* Check the value read against the value written       */
        if (check != value)
           {
                /* Check the value read against the original value, if  */
                /* the original value is not equal to the value written */
                if ((check != orig) && (value != orig))
                {
                        return(REG_ERR);
                }

                return(retc);
           }
        return(0);
   }
