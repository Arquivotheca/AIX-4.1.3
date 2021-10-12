static char sccsid[] = "src/bos/diag/tu/eth/g_netadd.c, tu_eth, bos411, 9428A410j 6/19/91 14:56:31";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: get_netadd
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

Function(s) Get Network Address

Module Name :  g_netadd.c
SCCS ID     :  1.4

Current Date:  6/13/91, 13:11:19
Newest Delta:  1/19/90, 16:24:48

                       >>> IMPORTANT NOTE <<<

hard_reset() MUST be called prior to calling this function due
to the problem with reading VPD with PAUSE/RESUME (10/03/89).
I could've put a call to hard_reset() in here, but one is being
done in the wrap() anyway (which is the only guy who calls this
function) so we don't need another in here which would slow us
down.

*****************************************************************************/
#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include "ethtst.h"

#ifdef debugg
extern void detrace();
#endif

int get_netadd (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int i;
	int rc;
	unsigned char   *ucp;
	unsigned char   network_address[NETADD_LEN];
	unsigned long   status;

	extern unsigned char pos_rd();	/* reads value from POS register */
	extern int pos_wr();		/* writes value to POS register */
	extern int set_pause();
	extern int set_resume();
	
	/*
	 * Check firmware version.  If > 7, then card needs
	 * PAUSE/RESUME of adapter cpu to read VPD.  Else,
	 * card doesn't!
	 */
	if (tucb_ptr->eth_htx_s.config_table[13] > 7)
	   {
		/*
		 * send PAUSE command to execute mailbox -
		 * temporary fix for gate array problem.  Will
		 * be possibly fixed in next pass of silicon so
		 * may get to remove this in the future....
		 */
		if (rc = set_pause(fdes, tucb_ptr))
			return(rc);
	   }

	memset(network_address, 0x00, NETADD_LEN);

	/*
	 * Read the 12th byte thru the 17th byte of VPD.
	 */
	ucp = network_address;
	for (i = 12; i < 18; i++)
	   {
		rc = pos_wr(fdes, 6, i, &status, tucb_ptr);
	        if (rc)
		   {
			if (tucb_ptr->eth_htx_s.config_table[13] > 7)
				(void) set_resume(fdes, tucb_ptr);
	        	return(POS6_WR_ERR);
		   }

		rc = pos_wr(fdes, 7, 0, &status, tucb_ptr);
		if (rc)
		   {
			if (tucb_ptr->eth_htx_s.config_table[13] > 7)
				(void) set_resume(fdes, tucb_ptr);
			return(POS7_WR_ERR);
		   }
		*ucp++ = pos_rd(fdes, 3, &status, tucb_ptr);
		if (status != 0)
		   {
			if (tucb_ptr->eth_htx_s.config_table[13] > 7)
				(void) set_resume(fdes, tucb_ptr);
			return(POS3_RD_ERR);
		   }

	   } /* endfor */

	if (pos_wr(fdes, 6, 0x00, &status, tucb_ptr))
        	return(POS6_WR_ERR);
	if (pos_wr(fdes, 7, 0x00, &status, tucb_ptr))
        	return(POS7_WR_ERR);

	if (tucb_ptr->eth_htx_s.config_table[13] > 7)
	   {
		if (rc = set_resume(fdes, tucb_ptr))
			return(rc);
	   }
	
	memcpy(tucb_ptr->eth_htx_s.net_addr, network_address, NETADD_LEN);
	return(0);
   }
