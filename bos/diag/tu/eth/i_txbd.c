static char sccsid[] = "src/bos/diag/tu/eth/i_txbd.c, tu_eth, bos411, 9428A410j 6/19/91 14:57:28";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: init_tx_bd
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

Function(s) Initialize Transmit Buffer Descriptor

Module Name :  i_txbd.c
SCCS ID     :  1.7

Current Date:  6/13/91, 13:11:19
Newest Delta:  1/19/90, 16:25:01

*****************************************************************************/
#include <stdio.h>
#include <errno.h>

#include "ethtst.h"	/* note that this also includes hxihtx.h */

#ifdef debugg
extern void detrace();
#endif

int init_tx_bd (fdes, dma_base_add, bufsize, tx_mb_sp, tx_bd_sp,
						former_bd_sp, tucb_ptr)
   int fdes;
   unsigned long dma_base_add;
   unsigned short bufsize;
   struct mailbox *tx_mb_sp;
   struct buffer_descriptor *tx_bd_sp;
   struct buffer_descriptor *former_bd_sp;
   TUTYPE *tucb_ptr;
   {
	int rc;
	unsigned char cval;
	unsigned short sval;
	unsigned short bd_address;
	unsigned long status;

	extern unsigned short swap();
	extern int smem_wr();
	extern int smem_rd();
	extern int wait_mb();
	extern int wait_bd();

	/*
	 * Make sure mailbox isn't busy
	 */
	if (rc = wait_mb(fdes, tx_mb_sp->a_status, 3, tucb_ptr))
	   {
#ifdef debugg
		detrace(1,"init_txbd:  TX mbox BUSY at beginning\n");
#endif
		if (rc == MB_BSY_ERR)
			rc = TXMB_BSY_ERR;
		else
			rc = TXMB_ERR_ERR;

		return(rc);
	   }

 	/*
	 * Get the address of the Transmit List (i.e. the first
	 * transmit buffer descriptor).
	 */
	if (rc = smem_rd(fdes, (unsigned int) tx_mb_sp->a_list_ptr, 2,
		(unsigned char *) &bd_address, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(1,"i_txbd:  reading tx_mb list pointer\n");
#endif
		return(TXMB_LIS_ERR);
	   }
	bd_address = swap(bd_address);


	/*
	 * now, we're going to save off all the addresses
	 * to this buffer descriptor in "former_bd_sp" since it will
	 * be needed later (to zero out the EL bit)
	 * to invoke transmission.
	 */
	former_bd_sp->a_ctrl_status = bd_address + 0;
	former_bd_sp->a_next_ptr    = bd_address + 2;
	former_bd_sp->a_count       = bd_address + 4;
	former_bd_sp->a_buf_lsw_ptr = bd_address + 6;
	former_bd_sp->a_buf_msw_ptr = bd_address + 8;


	/*
	 * Now, we need to get the address of the NEXT buffer descriptor
	 * in the Transmit List.  This is the one that we have
	 * to set up our packet stuff in.
	 */
	if (rc = smem_rd(fdes,(unsigned int) former_bd_sp->a_next_ptr, 2,
		(unsigned char *) &bd_address, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(1,"i_txbd:  former NEXT buffer desc.\n");
#endif
		return(TXBD_NX_RD_ERR);
	   }
	bd_address = swap(bd_address);

	tx_bd_sp->a_ctrl_status = bd_address + 0;
	tx_bd_sp->a_next_ptr    = bd_address + 2;
	tx_bd_sp->a_count       = bd_address + 4;
	tx_bd_sp->a_buf_lsw_ptr = bd_address + 6;
	tx_bd_sp->a_buf_msw_ptr = bd_address + 8;

	/*
	 * Now, we have to set up the pointers in the
	 * TX buffer descriptor to point to our
	 * DMA buffer that we allocated to hold our
	 * transmit packet data.
	 *
	 * Recall that we allocated one BIG DMA buffer
	 * with the first half to hold the transmit data
	 * and the second half to hold the receive data.
	 */

	/*
	 * We have to describe our 32-bit dma address into
	 * two (2) 16-bit addresses, so let's do the
	 * least-significant-word (LSW) first.
	 */
	sval = (unsigned short) (dma_base_add & 0x0000ffff);
	sval = swap(sval);
	if (rc = smem_wr(fdes,(unsigned int) tx_bd_sp->a_buf_lsw_ptr, 2,
		(unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
	detrace(1,"i_txbd:  Writing to tx lsw wrote sval = %x\n",sval);
#endif
		return(TXBD_LS_WR_ERR);
	   }

	/*
	 * okay, let's write the (most-significant-word) MSW word now.
	 */
	sval = (unsigned short) ((dma_base_add >> 16) & 0x0000ffff);
	sval = swap(sval);
	if (rc = smem_wr(fdes,(unsigned int) tx_bd_sp->a_buf_msw_ptr, 2,
		(unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
	detrace(1,"i_txbd:  Writing to tx msw wrote sval = %x\n",sval);
#endif
		return(TXBD_MS_WR_ERR);
	   }

	/*
	 * Set Tx Byte count in TX buffer descriptor
	 */
	sval = bufsize;
	sval = swap(sval);
	if (rc = smem_wr(fdes, (unsigned int) tx_bd_sp->a_count, 2,
		(unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
detrace(1,"i_txbd:  Writing to tx bd count wrote sval = %x\n",sval);
#endif
		return(TXBD_CO_WR_ERR);
	   }


	/*
	 * Clear the status in the TX buffer descriptor
	 */
	cval = 0x00;
	if (rc = smem_wr(fdes, (unsigned int) tx_bd_sp->a_ctrl_status,
		1, (unsigned char *) &cval, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(1,"i_txbd:  clearing NEXT TX bd status\n");
#endif
		return(TXBD_ST_WR_ERR);
	   }

	/*
	 * Mark this packet with the EL (end-of-list bit) and
	 * EOP (end-of-packet) bit.
	 */
	cval = 0xC0;
	if (rc = smem_wr(fdes, (unsigned int) tx_bd_sp->a_ctrl_status + 1,
		1, (unsigned char *) &cval, &status, tucb_ptr))
	   {
#ifdef debugg
	detrace(1,"i_txbd:  Writing EL/EOP 0x%01x to tx_bd ctrl\n",sval);
#endif
		return(TXBD_CT_WR_ERR);
	   }

	/*
	 * Make sure mailbox isn't busy
	 */
	if (rc = wait_mb(fdes, tx_mb_sp->a_status, 3, tucb_ptr))
	   {
#ifdef debugg
		detrace(1,"init_txbd:  TX mbox at end is BUSY\n");
#endif
		if (rc == MB_BSY_ERR)
			rc = TXMB_BSY_ERR;
		else
			rc = TXMB_ERR_ERR;
		return(rc);
	   }

	/*
	 * Now, let's clear the status of the 
	 * Transmit Mailbox
	 */
	sval = 0x0000;
	if (rc = smem_wr(fdes,(unsigned int) tx_mb_sp->a_status, 2,
		(unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
	detrace(1,"i_txbd:  failed Writing to clear tx mb stat\n");
#endif
		return(TXMB_WR_ERR);
	   }

	return(0);
   }
