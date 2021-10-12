static char sccsid[] = "src/bos/diag/tu/eth/st_eth.c, tu_eth, bos411, 9428A410j 6/19/91 14:58:00";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: start_eth
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

Function(s) Start Ethernet Adapter

Module Name :  st_eth.c
SCCS ID     :  1.9

Current Date:  6/13/91, 13:11:25
Newest Delta:  2/27/90, 14:36:14

*****************************************************************************/
#include <stdio.h>
#include <memory.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/entuser.h>
#include <errno.h>

#include "ethtst.h"	/* note that this also includes hxihtx.h */
#define MAX_START_ATTEMPTS	16
#define SLEEP_TIME		 1

#ifdef debugg
extern void detrace();
#endif

int start_eth (fdes, sess_sp, tucb_ptr)
   int fdes;
   struct session_blk *sess_sp;
   TUTYPE *tucb_ptr;
   {
	register int i;
	int rc;
	unsigned long status;
	struct status_block stat_s;
	extern int get_atype();
	extern int mktu_rc();
	extern int hio_rampg_wr();

	/*
	 * start up the adapter with the session block
	 * PREVIOUSLY initialized (pointed to by sess_sp).
	 */
	if (ioctl(fdes, CIO_START, sess_sp) < 0)
	   {
		if ((tucb_ptr->header.mfg == INVOKED_BY_HTX) &&
		    (tucb_ptr->eth_htx_s.htx_sp != NULL))
			(tucb_ptr->eth_htx_s.htx_sp->bad_others)++;
		
		if (errno == EIO)
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
							START_ERR));

		return(mktu_rc(tucb_ptr->header.tu, SYS_ERR, errno));
	   }
	if ((tucb_ptr->header.mfg == INVOKED_BY_HTX) &&
	    (tucb_ptr->eth_htx_s.htx_sp != NULL))
		(tucb_ptr->eth_htx_s.htx_sp->good_others)++;
	
	for (i = 0; i < MAX_START_ATTEMPTS; i++)
	   {
		/*
		 * insure that adapter started properly AND
		 * grab the network address from the status block
		 */
		if (ioctl(fdes, CIO_GET_STAT, &stat_s) < 0)
		   {
			if ((tucb_ptr->header.mfg == INVOKED_BY_HTX) &&
			    (tucb_ptr->eth_htx_s.htx_sp != NULL))
				(tucb_ptr->eth_htx_s.htx_sp->bad_others)++;
			
			if (errno == EIO)
				return(mktu_rc(tucb_ptr->header.tu,
						LOG_ERR, START_STAT_ERR));

			return(mktu_rc(tucb_ptr->header.tu, SYS_ERR, errno));
		   }
		if ((tucb_ptr->header.mfg == INVOKED_BY_HTX) &&
		    (tucb_ptr->eth_htx_s.htx_sp != NULL))
			(tucb_ptr->eth_htx_s.htx_sp->good_others)++;
		
		if ((stat_s.code & 0x0000ffff) == CIO_START_DONE)
			break;

		sleep(SLEEP_TIME);
	   }
		
	if (i == MAX_START_ATTEMPTS)
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, START_TIME_ERR));

	if (stat_s.option[0] != CIO_OK)
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, START_BSTAT_ERR));
	
	/*
	 * NOTE!!!
	 *
	 * As of 10/04/89, no longer grab network address from
	 * here (start completion).  When needed (wrap test), the
	 * individual TU will get it directly from the VPD.
	 *

	memcpy(&tucb_ptr->eth_htx_s.net_addr[0], &stat_s.option[2], 4);
	memcpy(&tucb_ptr->eth_htx_s.net_addr[4], &stat_s.option[3], 2);

	 *
	 */
	
	/*
	 * Now, insure that our RAM page offset is zero - i.e.
	 * that are shared memory window starts at the base RAM
	 * address of the card.
	 */
	if (hio_rampg_wr(fdes, 0x00, &status, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RRAM_WR_ERR));
	   }

	/*
	 * Get the adapter type - either PS2 or AT.
	 */
	if (rc = get_atype(fdes, tucb_ptr))
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
	   }

	return(0);
   }
