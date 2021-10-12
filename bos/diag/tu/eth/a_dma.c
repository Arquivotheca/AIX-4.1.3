static char sccsid[] = "src/bos/diag/tu/eth/a_dma.c, tu_eth, bos411, 9428A410j 6/19/91 14:56:23";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: access_dma
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

Function(s) Access DMA

Module Name :  a_dma.c
SCCS ID     :  1.10

Current Date:  6/13/91, 13:11:16 
Newest Delta:  2/27/90, 14:35:24

Function used to LOCK-WRITE/UNLOCK-READ/FREE DMA buffer.  We do this because
we have to describe a DMA buffer (as opposed to a regular buffer in
our own user space) for the adapter (as bus master) to read/write
the packet data.

                            >>> IMPORTANT <<<

Initially, after allocating a DMA buffer, we copied in/out our user data
while specifying just the LENGTH of the user data rather than than using
the entire allocated DMA buffer.  This may have caused some problems
with the DMA kernel services, so as of 09/27/89, we're trying writing/reading
with the exact full length of the allocated buffer.

*****************************************************************************/
#include <stdio.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/entuser.h>
#include <sys/sysdma.h>
#include <errno.h>

#include "ethtst.h"	/* note that this also includes hxihtx.h */

#ifdef debugg
extern void detrace();
#endif
/*****************************************************************************

calc_page_offset

Function takes address given by 'base_alloc_address' and RETURNS
THE OFFSET to the NEXT page boundary address.  Note this could be
zero if the given 'base_alloc_address' is already on a page boundary.

*****************************************************************************/

unsigned long calc_page_offset (base_alloc_address)
   unsigned long base_alloc_address;
   {
	unsigned long long_address,
		      long_mask;

	/*
	 * form the mask which will be AND'd to the address
	 * calculation to force a page boundary.
	 */
	long_mask = DMA_PSIZE - 1;
	long_mask = ~long_mask;

	long_address = base_alloc_address;

	/*
	 * Now, we AND our calculated address (beyond first page)
	 * with the mask to force page boundary alignment.
	 */
	long_address = (long_address + (DMA_PSIZE - 1)) & long_mask;
	return(long_address - base_alloc_address);
   }
/*****************************************************************************

access_dma

IF MAKING ANY CHANGES IN HERE, YOU HAD BETTER READ ME FIRST!!!
THIS DETAILS ON HOW THE WHOLE DMA SCENARIO WORKS!!!
IT GETS CONFUSING, BUT READ ALL THE WAY THROUGH!

  1)  First, we cannot give the adapter an address here in
      our own user space, so we must ask the device driver
      to allocate a DMA space for us in the kernel.  This
      requires several steps.

  2)  Before working with the device driver, we must perform
      some work in a buffer in our own user space.  What we
      do is allocate a big buffer (via "malloc()") and split
      it into two parts - one side for transmit and the other
      for receive.

  3)  Initially, we only had to "malloc()" enough space for
      our transmit and receive.  However, due to a change
      in the device driver, we have some extra work to do
      EXPLAINED LATER IN STEP 9).

      Once we've copied in our transmit data to the first half
      of our big buffer and assigned the pointer in the special
      ioctl (p_user) structure to it (along with filling in the length),
      we then issue the ioctl to LOCK_DMA.

  4)  The driver then LOCKS the page of memory that our buffer
      resides in!  YOU CANNOT IN ANY WAY, SHAPE, OR FORM 
      ACCESS THIS MEMORY UNTIL YOU ISSUE A UNLOCK_DMA!!!!!!!!!
      UNEXPECTED RESULTS FROM FAILURES TO HANGING THE MACHINE
      COULD OCCUR!!!

  5)  From the ioctl, you access the address returned by the
      device driver in the structure (p_bus) which gives us the
      corresponding DMA kernel address which we'll supply to
      the adapter for transmitting (after leaving this
      function).  Also, we'll take this kernel address and add on offset
      (the half way point) to give to the adapter as our receive
      DMA kernel address.

  6)  We then leave here and get our adapter transmitting/receiving.
      After getting a receipt confirmation from the adapter, we
      come back here with op as UNLOCK_DMA.

  7)  During step 6), the following occurred.  The device driver
      used our LOCKED buffer and moved the data to the kernel
      DMA address so the adapter could transmit it.  Then, the
      adapter as bus master grabbed the data.  On receipt of
      data, the adapter DMA'd receive data to the our receive
      DMA area = DMA kernel address (from ioctl) + 1/2way offset.

  8)  When we do an UNLOCK_DMA, the device driver insures that the
      receive data in the kernel's DMA address space is copied
      into our space as pointed by p_user in the ioctl structure.

  9)  SPECIAL CONSIDERATIONS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      Note the malloc size during our LOCK_DMA.  We used to
      be able to specify buf_size as (2 * packet_size), i.e.
      enough for our transmit and the receive.  Now since a change
      in the device driver we have to use care due to STEP 4).  Note
      that the device driver LOCKS THE ENTIRE MEMORY PAGE associated with
      our malloc'd space.  If our buffer is less than a memory page,
      then the compiler MOST PROBABLY has other variables on
      that page.  If we lock the page, then we cannot access
      those other variables!!!  To get around this, we now allocate
      two pages worth of memory.  This way, we can calculate
      a full, single page on a page boundary which will then
      be the only space locked.  This prevents other variables
      from getting locked and saves us!

*****************************************************************************/

int access_dma (fdes, op, rbuf, buf_size, packet_size, dma_base_add, tucb_ptr)
   int fdes;
   int op;
   unsigned char *rbuf;
   unsigned long buf_size;
   unsigned long packet_size;
   unsigned long *dma_base_add;
   TUTYPE *tucb_ptr;
   {
	int rc;
	static unsigned long page_offset;
	static int lock_flg = 0;
	static ent_dma_buf_t dma_s;
	unsigned long save_mem;
	struct htx_data *htx_sp;
	static char *cp = NULL;
	char *malloc();


#ifdef debugg
	int i;
#endif

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->eth_htx_s.htx_sp;

	if ((op != LOCK_DMA) &&
	    (!lock_flg))
		return(DMA_NOTA_ERR);

	switch(op)
	   {
		case LOCK_DMA:
			/*
			 * allocate/lock a DMA buffer.  Note
			 * that buf_size should have been passed
			 * in as DMA_PSIZE * 2.
			 */
			if (cp == NULL)
			   {
				if ((cp = malloc(buf_size)) == NULL)
				   {
#ifdef debugg
					detrace(1,"a_dma:  malloc failed\n");
#endif
					return(DMA_MAL_ERR);
				   }
			    /*
			     * Now find the offset to the next PAGE boundary
			     * from our malloc'd "cp".  This could be zero
			     * if we luckily received a buffer already
			     * on a PAGE boundary.
			     */
			page_offset = calc_page_offset((unsigned long) cp);
			   }

			memset(cp, 0, buf_size);

			/*
			 * Copy in our transmit data to the address
			 * we've calculated as being on a PAGE boundary.
			 */
			memcpy(cp + page_offset, rbuf, packet_size);
			dma_s.p_user = cp + page_offset;

			/*
			 * Note here that we're only specify a length
			 * of one PAGE.  With our setups, we'll then
			 * have only one PAGE on a PAGE boundary locked
			 * up.
			 */
			dma_s.length = DMA_PSIZE;
#ifdef debugg
	detrace(0,"a_dma:  malloc'd %d (0x%x) bytes\n", buf_size, buf_size);
	detrace(0,"        cp = 0x%x, page_offset = 0x%x, cp+offs = 0x%x\n",
					cp, page_offset, cp + page_offset);
#endif
			rc = ioctl(fdes, ENT_LOCK_DMA, &dma_s);
			if (rc)
			   {
				if (htx_sp != NULL)
					(htx_sp->bad_others)++;
/*
detrace(0,"a_dma:  ENT_LOCK_DMA failed...\n");
detrace(1,"  p_user = 0x%08x, length = %d (0x%x), p_bus = 0x%08x, errno = %d\n",
	dma_s.p_user, dma_s.length, dma_s.length, dma_s.p_bus, errno);
*/

				free(cp);
				cp = NULL;
				return(DMA_ALOC_ERR);
			   }
			if (htx_sp != NULL)
				(htx_sp->good_others)++;
			*dma_base_add = dma_s.p_bus;
#ifdef debugg
			detrace(1,"a_dma:  dma base add (p_bus): 0x%08x\n",
				*dma_base_add);
#endif
			lock_flg = 1;
			break;

		case UNLOCK_DMA:
			/*
			 * the adapter DMA's data to the DMA buffer
			 * we allocated, so we copy from there
			 * into our own user space.
			 */

			/*
			 * make sure area is ALL clean!
			 */
			memset(rbuf, 0, packet_size);
#ifdef debugg
			detrace(0,"a_dma:  p_bus is at 0x%08x\n", dma_s.p_bus);
#endif
			rc = ioctl(fdes, ENT_UNLOCK_DMA, &dma_s);
			if (rc)
			   {
				if (htx_sp != NULL)
					(htx_sp->bad_others)++;
				free(cp);
				cp = NULL;
				return(DMA_RD_ERR);
			   }
			if (htx_sp != NULL)
				(htx_sp->good_others)++;
			
#ifdef debugg
	detrace(1,"a_dma:  copying from cp+page_off+psize/2 = 0x%x\n",
				cp + page_offset + (DMA_PSIZE / 2));
#endif
			/*
			 * Recall that we split our buffer into
			 * two parts, so in order to copy our receive
			 * data, we copy from the half way point
			 * in our PAGE on a PAGE boundary.
			 */
			memcpy(rbuf, cp + page_offset + (DMA_PSIZE / 2),
							packet_size);
			lock_flg = 0;
			break;

		case FREE_DMA:
			/*
			 * Free up the DMA buffer.
			 *
			 * Called on error type situations.
			 */

			 /*
			  * probably don't need, should be saved

			dma_s.length = DMA_PSIZE;

			*/

			rc = ioctl(fdes, ENT_UNLOCK_DMA, &dma_s);
			if (rc)
			   {
				if (htx_sp != NULL)
					(htx_sp->bad_others)++;
			
				free(cp);
				cp = NULL;
				lock_flg = 0;
				return(DMA_FREE_ERR);
			   }
			free(cp);
			cp = NULL;
			if (htx_sp != NULL)
				(htx_sp->good_others)++;
			lock_flg = 0;
			break;

		default:
			return(DMA_UNK_ERR);
	   }
	return(0);
   }
