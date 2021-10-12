static char sccsid[] = "src/bos/diag/tu/eth/s_resume.c, tu_eth, bos411, 9428A410j 6/19/91 14:59:51";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: set_resume
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

Function(s) Set RESUME command

Module Name :  s_resume.c
SCCS ID     :  1.2

Current Date:  6/13/91, 13:11:23
Newest Delta:  1/19/90, 16:28:14

*****************************************************************************/
#include <stdio.h>
#include <errno.h>

#include "ethtst.h"	/* note that this also includes hxihtx.h */

#ifdef debugg
extern void detrace();
#endif

int set_resume (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	short unsigned sval;
	unsigned long status;

	extern int smem_wr();

	/*
	 * write only parameter (resume value - all zeroes)
	 * for parm 0 in the execute mailbox.
	 */
	sval = 0x0000;
#ifdef debugg
	detrace(0,"set_resume:  resume value is 0x%04x\n", sval);
#endif
	if (rc = smem_wr(fdes,
		(unsigned int) tucb_ptr->eth_htx_s.exec_mbox_offset + 2,
		2, (unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"set_resume:  smem_wr failed!\n");
#endif
		return(SRES_P_ERR);
	   }

	return(0);
   }
