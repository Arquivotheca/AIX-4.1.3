static char sccsid[] = "src/bos/diag/tu/eth/setup_rx.c, tu_eth, bos411, 9428A410j 6/19/91 15:00:33";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: setup_rx
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

Function(s) Set up Receive

Module Name :  setup_rx.c
SCCS ID     :  1.6

Current Date:  6/13/91, 13:11:24
Newest Delta:  2/14/90, 18:49:59

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

*****************************************************************************/
#include <stdio.h>
#include <sys/sysdma.h>
#include <errno.h>

#include "ethtst.h"	/* note that this also includes hxihtx.h */

#ifdef debugg
extern void detrace();
#endif

int setup_rx (fdes, mb_sp, bd_sp, dma_base_add, tucb_ptr)
   int fdes;
   struct mailbox *mb_sp;
   struct buffer_descriptor *bd_sp;
   unsigned long dma_base_add;
   TUTYPE *tucb_ptr;
   {
	int rc;
	unsigned short sval;
	unsigned long status;
	unsigned short bd_address;
	unsigned long rx_dma_base_add;
	extern unsigned short swap();
	extern int smem_wr();

#ifdef debugg
	detrace(1,"setup_rx:  dma_base_add 0x%08x\n", dma_base_add);
#endif
	/*
	 * get address of Receive Mailbox status, and
	 *     address of Receive List Pointer.
	 */
	mb_sp->a_status = tucb_ptr->eth_htx_s.config_table[3];
	mb_sp->a_list_ptr = mb_sp->a_status + 2;

#ifdef debugg
	detrace(0,"setup_rx:  rx mb status   at 0x%02x\n", mb_sp->a_status);
	detrace(0,"           rx mb list_ptr at 0x%02x\n", mb_sp->a_list_ptr);
#endif

       /*
	* Clear the Status (offset 0) in the Receive mailbox
	*/
	sval = 0x0000;
	if (rc = smem_wr(fdes, (unsigned int) mb_sp->a_status, 2,
		(unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(1,
	"setup_rx:  Writing to rx mb status, wrote = 0x%x\n",
			sval);
#endif
		return(RXMB_WR_ERR);
	   }

	/*
	 * Now, we (the host) are responsible for setting the 
	 * Pointer to Receive List (offset 2), so let's grab the address
	 * of the first rec'v buffer descriptor (gained from our config_table),
	 * save off the addresses of each item within the buffer descriptor.
	 */
	bd_address = tucb_ptr->eth_htx_s.config_table[11];
	bd_sp->a_ctrl_status = bd_address + 0;
	bd_sp->a_next_ptr    = bd_address + 2;
	bd_sp->a_count       = bd_address + 4;
	bd_sp->a_buf_lsw_ptr = bd_address + 6;
	bd_sp->a_buf_msw_ptr = bd_address + 8;

#ifdef debugg
	detrace(0,"setup_rx:  rx bd status at   0x%02x\n",
					bd_sp->a_ctrl_status);
#endif
	/*
	 * Okay, let's set the address of the first buffer descriptor
	 * (i.e. the front of our Receive List) in the 
	 * Receive List pointer of the Receive Mailbox.
	 */
	sval = swap(bd_address);
	if (rc = smem_wr(fdes,(unsigned int) mb_sp->a_list_ptr, 2,
		(unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(1,
		  "setup_rx:  Writing to rx mb list ptr sval=0x%x\n",
			sval);
#endif
		return(RXMB_LISW_ERR);
	   }
#ifdef debugg
	detrace(0,"setup_rx:  Wrote swapped 0x%08x to rx mb list ptr\n",
			sval);
#endif
	
	/*
	 * Now, we need to set the address of our DMA buffer
	 * where the adapter can put the packet data.
	 * Since we will only be receiving one packet,
	 * we'll only use this buffer descriptor 
	 * so we can go ahead and set up the DMA buffer address.
	 *
	 * Recall that we allocated a big DMA buf that we
	 * are going to split into two pieces:  the first half
	 * is for transmit, and the second half for receive.
	 *
	 * For TONS of documentation/comments on this,
	 * see the "a_dma.c" file.
	 */
	rx_dma_base_add = dma_base_add + (DMA_PSIZE / 2);

#ifdef debugg
	detrace(1,"setup_rx:  rx_dma_base_add formed at 0x%x\n",
		rx_dma_base_add);
#endif

	/*
	 * We have to describe our 32-bit dma address into
	 * two (2) 16-bit addresses, so let's do the
	 * least-significant-word (LSW) first.
	 */
	sval = (unsigned short) (rx_dma_base_add & 0x0000ffff);
	sval = swap(sval);
	if (rc = smem_wr(fdes,(unsigned int) bd_sp->a_buf_lsw_ptr, 2,
		(unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(1,
		  "setup_rx:  Writing to rx_bd_lsw with sval=0x%x\n",
			sval);
#endif
		return(RXBD_LS_WR_ERR);
	   }
#ifdef debugg
	detrace(0,"setup_rx:  Wrote (swapped) 0x%02x for bd lsw\n",
			sval);
#endif

	/*
	 * Okay, let's do the most-significant-word (MSW) now.
	 */
	sval = (unsigned short) ((rx_dma_base_add >> 16) & 0x0000ffff);
	sval = swap(sval);
	if (rc = smem_wr(fdes,(unsigned int) bd_sp->a_buf_msw_ptr, 2,
		(unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(1,
		  "setup_rx:  Writing to rx_bd_msw with sval=0x%x\n",
			sval);
#endif
		return(RXBD_MS_WR_ERR);
	   }
#ifdef debugg
	detrace(0,"setup_rx:  Wrote (swapped) 0x%02x for bd msw\n",
			sval);
#endif

	/*
	 * Clear the Status Word in the Receive Buffer Descriptor
	 */
	sval = 0x0000;
	if (rc = smem_wr(fdes,(unsigned int) bd_sp->a_ctrl_status, 2,
		(unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
detrace(1,"setup_rx:  Writing to buf_des1 wrote sval = 0x%x\n",sval);
#endif
		return(RXBD_ST_WR_ERR);
	   }

	/*
	 * Set the EL (end-of-list) bit in the status portion 
	 * of the buffer descriptor
	 */
	sval = 0x4000;
	sval = swap(sval);
	if (rc = smem_wr(fdes,(unsigned int) bd_sp->a_ctrl_status, 2,
		(unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
detrace(1,"setup_rx:  Writing to rx bd stat wrote sval = 0x%x\n",sval);
#endif
		return(RXBD_CT_WR_ERR);
	   }
	
	return(0);
   }
