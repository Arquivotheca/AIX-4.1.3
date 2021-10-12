static char sccsid[] = "src/bos/diag/tu/eth/wait_mb.c, tu_eth, bos411, 9428A410j 6/19/91 14:58:14";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: wait_mb
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

Function(s) Wait for Mailbox

Module Name :  wait_mb.c
SCCS ID     :  1.5

Current Date:  6/13/91, 13:11:30
Newest Delta:  1/19/90, 16:32:10

*****************************************************************************/
#include <stdio.h>
#include <time.h>
#include <errno.h>

#include "ethtst.h"	/* note that this also includes hxihtx.h */

#ifdef debugg
extern void detrace();
#endif

int wait_mb (fdes, mb_address, time_offset, tucb_ptr)
   int fdes;
   unsigned short mb_address;
   unsigned long time_offset;
   TUTYPE *tucb_ptr;
   {
	int rc;
	unsigned long c_time, t_time;

	short unsigned sval;
	unsigned long status;

	extern unsigned short swap();
	extern int smem_rd();

	if (rc = smem_rd(fdes, (unsigned int) mb_address, 2,
		(unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(1,"wait_mb:  read of mb_address failed\n");
#endif
		return(MB_RD_ERR);
	   }
	sval = swap(sval);
	t_time = time((long *) 0) + time_offset;
	while ((sval & 0x4000) &&
	       ((c_time = time((long *) 0)) < t_time) )
	   {
		if (rc = smem_rd(fdes, (unsigned int) mb_address, 2,
			(unsigned char *) &sval, &status, tucb_ptr))
		   {
#ifdef debugg
		detrace(1,"wait_mb:  read of mb_address failed\n");
#endif
			return(MB_RD_ERR);
		   }
		sval = swap(sval);
	   }
	if (sval & 0x4000)
	   {
#ifdef debugg
		detrace(1,"wait_mb:  timed out - status 0x%04x\n", sval);
#endif
		return(MB_BSY_ERR);
	   }

	return(0);
   }
