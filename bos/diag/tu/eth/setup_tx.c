static char sccsid[] = "src/bos/diag/tu/eth/setup_tx.c, tu_eth, bos411, 9428A410j 6/19/91 15:01:27";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: setup_tx
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
 */
/*****************************************************************************

Function(s) Set up Transmit

Module Name :  setup_tx.c
SCCS ID     :  1.5

Current Date:  6/13/91, 13:11:24
Newest Delta:  1/19/90, 16:28:37

IMPORTANT NOTE!!!
The "config_table" value in the tucb_ptr is set by a
call to hard_reset().  Thus, hard_reset() must have been
invoked previously with success to insure a correct value
here.

Also, note that while we DID perform BYTE SWAPPING of
the addresses written into our config_table, we do NOT
swap them prior to calling smem_wr/rd IF WE ARE USING
THEM TO SPECIFY THE READ/WRITE ADDRESS (the device driver
does this for us).  We DO SWAP if we are writing it as DATA though!

Note that we do NOT do any setup's on DMA buffer addresses
in here since this routine is only called once.  You have
to set up the DMA addresses EVERY time since you get a
new buffer descriptor each you invoke a setup/transmit.

*****************************************************************************/
#include <stdio.h>
#include <errno.h>

#include "ethtst.h"	/* note that this also includes hxihtx.h */

#ifdef debugg
extern void detrace();
#endif

int setup_tx (fdes, mb_sp, tucb_ptr)
   int fdes;
   struct mailbox *mb_sp;
   TUTYPE *tucb_ptr;
   {
	int rc;

	unsigned short sval;
	unsigned long status;
	extern unsigned short swap();
	extern int smem_wr();

	/*
	 * get address of Transmit Mailbox status, and
	 *     address of Transmit List Pointer.
	 */
	mb_sp->a_status = tucb_ptr->eth_htx_s.config_table[4];
	mb_sp->a_list_ptr = mb_sp->a_status + 2;

       /*
	* Clear the Status (offset 0) in the Transmit mailbox
	*/
	sval = 0x0000;
	if (rc = smem_wr(fdes, (unsigned int) mb_sp->a_status, 2,
		(unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(1,
	"setup_tx:  Writing to tx mb status wrote 0x%x\n",
			sval);
#endif
		return(TXMB_WR_ERR);
	   }

	return(0);
   }
