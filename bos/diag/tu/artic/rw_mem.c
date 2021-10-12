static char sccsid[] = "@(#)95  1.3  src/bos/diag/tu/artic/rw_mem.c, tu_artic, bos411, 9428A410j 8/19/93 17:49:18";
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
 * FUNCTIONS: dh_diag_mem_read, dh_diag_mem_write
 *
 */
#include <stdio.h>
#include <artictst.h>
typedef unsigned char byte;

/*
 * NAME: DH_DIAG_MEM_READ
 *
 * FUNCTION: Read one byte from the adapter's RAM.
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will issue the Device Driver's memory read IOCTL
 *          via the C Library icareadmem call.
 *
 * RETURNS: A zero (0) for no error or a non-zero for error condition
 *
 */

int dh_diag_mem_read (fdes, tucb_ptr, card_page, card_offset, data_pt)
   int fdes;
   TUTYPE *tucb_ptr;
   byte card_page;
   unsigned short card_offset;
   byte *data_pt;
   {
        unsigned short rc;

        rc = icareadmem(fdes,1, card_page,card_offset,0xFF,data_pt);
        if ( rc )
           {
                return((int) rc);
           }

        return(0);
   }
/*
 * NAME: DH_DIAG_MEM_WRITE
 *
 * FUNCTION: Write one byte to the adapter's RAM.
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will issue the Device Driver's memory write IOCTL
 *          via the C Library icareadmem call.
 *
 * RETURNS: A zero (0) for no error or a non-zero for error condition
 *
 */
int dh_diag_mem_write (fdes, tucb_ptr, card_page, card_offset, data_pt)
   int fdes;
   TUTYPE *tucb_ptr;
   byte card_page;
   unsigned short card_offset;
   byte *data_pt;
   {
        unsigned short rc;

        rc = icawritemem(fdes, 1,card_page,card_offset,0xFF,data_pt);
        if ( rc )
           {
                return((int) rc);
           }

        return(0);
   }
