static char sccsid[] = "@(#)55  1.1  src/bos/diag/tu/eth/tu012.c, tu_eth, bos411, 9428A410j 5/12/92 13:20:46";
/*
 * COMPONENT_NAME: (ETHERTU) Ethernet Test Unit
 *
 * FUNCTIONS: hd_rst_int (interrupts enabled), tu012
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************

Function(s) Test Unit 012 - Hard Reset Test w/interrupts enabled

*****************************************************************************/
#include <stdio.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <errno.h>
#include "ethtst.h"

#ifdef debugg
extern void detrace();
#endif


/*****************************************************************************

hd_rst_int

*****************************************************************************/


int hd_rst_int (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	unsigned char i,p0,p1;
	unsigned char sval;
	unsigned long status; 
	struct htx_data *htx_sp;
	
	extern int hio_ctrl_wr();
	extern unsigned char hio_ctrl_rd();
	extern unsigned char hio_cmd_rd();
	extern unsigned char hio_cmd_rd_q();
	extern int mktu_rc();
	extern unsigned char hio_parity_rd();
	extern int hio_parity_wr();

	extern int pos_wr();
	extern unsigned char pos_rd();

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->eth_htx_s.htx_sp;

#ifdef debugg
	detrace(0,"IN the Interrupt Test Routine\n"); 
#endif

	/*
	 * specify DIX connector via POS reg 
	 * to turn off transceiver circuitry which could
	 * catch external noise and affect the POST internal wrap.
	 */
	p0 = pos_rd(fdes, 4, &status, tucb_ptr);
	if (status)
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS4_RD_ERR));
	   }

	/*
	 * bit 0 is connector, (0=DIX, 1=BNC)
	 */
	p0 &= 0xfd;
	if (pos_wr(fdes, 4, p0, &status, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS4_WR_ERR));
	   }
	
	/*
	 * Clear the command queue in which the device driver stores
	 * command register values upon receiving interrupts.
	 */
	for (i = 0; i < 32; i++)
	   {
		p0 = hio_cmd_rd_q(fdes, &status, tucb_ptr);
		if (status)
		   {
			/*
			 * Check status indicating failure of reading
			 * command queue was due to being empty, else err
			 */
			if (status == CCC_QUE_EMPTY)
				break;
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				QCMD_RD_ERR));
		   }
	   }
	
	/*
	 * Make sure that we were able to clear
	 * the command queue so that our subsequent reads
	 * after the reset will contain expected values.
	 */
	if (status != CCC_QUE_EMPTY)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				QCMD_EMP_ERR));
	    }

	/*
	 * Set ATTN & RESET on
	 */
	if (hio_ctrl_wr(fdes, 0xc0, &status, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RCTL_WR_ERR));
	   }

	p1 = hio_ctrl_rd(fdes, &status, tucb_ptr);
	if (status != 0)
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RCTL_RD_ERR));
	   }

	/*
	 * Is ATTN & RESET on ?
	 */
	if ((p1 & 0xc0) != 0xc0)
	   {
#ifdef debugg
		detrace(1,"hard_reset:  p1 got %x (hex) after hio_ctrl_wr/rd of 0xc0\n", p1);
#endif
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RCTL_CMP_ERR));
	   }

	/*
	 * allow adapter to sit in RESET state for a sec...
	 */
	sleep(1);

	/*
	 * Clear ATTN & RESET, but set interrupts enabled so that
	 * the adapter will interrupt the driver who will save off
	 * the interrupt info from the command register in the 
	 * command queue.
	 */
	if (hio_ctrl_wr(fdes, 0x04, &status, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RCTL_WR_ERR));
	   }

	/*
	 * sleep a couple of seconds to allow
	 * adapter to go through reset sequence.
	 */
	sleep(2);

	/*
	 * Read the queue to see if we have received an interrupt
	 * if the queue is not empty will indicate that an interrupt
	 * has been received
	 */

	p1 = hio_cmd_rd_q(fdes, &status, tucb_ptr);
	if (status != 0)
	   {
		/*
		 * Check if command queue emptied before ever
		 * getting the reset status.
		 */
		if (status == CCC_QUE_EMPTY)
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				QCMD_EMP2_ERR));

		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				QCMD_RD_ERR));
	   }

	/*
	 * Now, since we do not care if the adapter actually
	 * resets, we no longer need to worry about the driver  
	 * servicing interrupts for us, so let's disable
	 * interrupts and just poll the adapter directly when
	 * we need to do so.
	 */

	if (hio_ctrl_wr(fdes, 0x00, &status, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RCTL_WR_ERR));
	   }

        /*
	 * Disable Parity in Parity Register since it
	 * now defaults to turn on after hard reset.
	 */
	sval = hio_parity_rd(fdes, &status, tucb_ptr);
	if (status)
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RPAR_RD_ERR));
	   }	
	sval = sval & 0xfe; 
	if (hio_parity_wr(fdes, sval, &status, tucb_ptr))
	   {
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RPAR_WR_ERR));
	   }


	/*
	 * Get adapter's execute mailbox offset.  If we've made it this
	 * far, then the driver should still have this info stored
	 * in the command queue.
	 */
	for (i = 0; i < 32; i+=8 )
	   {
		p0 = hio_cmd_rd_q(fdes, &status, tucb_ptr);
		if (status != 0)
		   {
#ifdef debugg
			detrace(1," Command Read Q Status = %x\n",status);
#endif
			if (status == CCC_QUE_EMPTY)
				return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
					QCMD_EMP3_ERR));
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				RCMD_RD_ERR));
		   }

	   }  /* End for loop */
	
	return(0);
   }

/*****************************************************************************

tu012

*****************************************************************************/

int tu012 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	return(hd_rst_int(fdes, tucb_ptr));
   }
