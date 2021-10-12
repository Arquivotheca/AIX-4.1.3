static char sccsid[] = "@(#)45        1.2.1.3  src/bos/diag/tu/artic/tu018.c, tu_artic, bos41J, 9508A 2/15/95 17:51:16";
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
 * FUNCTIONS: general_reg, tu018
 *
 */

#include <stdio.h>
#include <artictst.h>
typedef unsigned char byte;

/*
 * NAME: General Register Test
 *
 * FUNCTION: Check the ID register on the Gate Array
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will verify the Gate Array ID register.
 *
 * RETURNS: A zero (0) for successful completion or non-zero for error.
 *
 */

int generalreg (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int rc;      /* return code */
        int i;       /* loop index  */
        unsigned char value;
        int type;
        static unsigned char contender4 = 0x81;
        static unsigned char contender5 = 0x82;
        static unsigned char sstic2 = 0x30;
        static unsigned char sstic3 = 0xC0;

        extern int outby();
        extern int dh_diag_io_read();

        if (icagetadaptype(fdes, &type))
           return(DRV_ERR);

        if ((rc=outby(fdes, tucb_ptr, adapter_PTRREG,
                      0x0F,CAD_ER1)) != 0)
                   return(rc);

        if ((rc = dh_diag_io_read(fdes, tucb_ptr,
                                 (byte) adapter_DREG, &value)) != 0)
                   return(DDINPB6);

        if (IS_PORTMASTER(type) || IS_SP5(type))
        {
           if ((value != contender4) &&
               (value != contender5 ))
           {
                  return(COMREG_ER);
           }
        }
        else
        {
           if (IS_GALE(type) || IS_MP(type))
           {
              if (value != sstic2)
              {
                     return(COMREG_ER);
              }
           }
           else
           {
              if (value != sstic3)
              {
                     return(COMREG_ER);
              }
           }
        }

        return(0);
   }

/*
 * NAME: tu018
 *
 * FUNCTION: General Register Test
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This function will check power up value and bit
 *          functionality of  registers in the SSTIC (Gate Array).
 *          This test will write a 5555 then an AAAA to each of the
 *          registers making sure none of the bits are stuck.
 *
 * RETURNS: The return code from the General Register tests.
 *
 */

int tu018 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        return(generalreg(fdes, tucb_ptr));
   }
