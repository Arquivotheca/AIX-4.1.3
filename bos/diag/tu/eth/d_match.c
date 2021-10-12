static char sccsid[] = "src/bos/diag/tu/eth/d_match.c, tu_eth, bos411, 9428A410j 6/19/91 14:58:24";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: disable_match_filter
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

Function(s) Disable Pattern Match Filter

Module Name :  d_match.c
SCCS ID     :  1.6

Current Date:  6/13/91, 13:11:17
Newest Delta:  1/19/90, 16:23:48

*****************************************************************************/
#include <stdio.h>
#include <errno.h>

#include "ethtst.h"	/* note that this also includes hxihtx.h */

#ifdef debugg
extern void detrace();
#endif

int disable_match_filter (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	short unsigned val;
	unsigned long status;

	extern unsigned short swap();
	extern int smem_wr();
	extern int execute_cmd();

	/*
	 * write "set receive pattern match filter" command
	 * to the command word of the execute mailbox.
	 */
	val = 0x0003;
	val = swap(val);
	if (rc = smem_wr(fdes,
		(unsigned int) tucb_ptr->eth_htx_s.exec_mbox_offset,
		2, (unsigned char *) &val, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"disable_match_filter:  smem_wr failed!\n");
#endif
		return(DMFIL_C_ERR);
	   }
	/*
	 * write only parameter (zero count for disabling matching)
	 * for parm 1 in the execute mailbox.
	 */
	val = 0x0000;
	if (rc = smem_wr(fdes,
		(unsigned int) tucb_ptr->eth_htx_s.exec_mbox_offset + 4,
		2, (unsigned char *) &val, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"disable_match_filter:  smem_wr failed!\n");
#endif
		return(DMFIL_P_ERR);
	   }

	if (rc = execute_cmd(fdes, 0x40, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"disable_match_filter:  execute_cmd failed!\n");
#endif
		return(DMFIL_EX_ERR);
	   }

	return(0);
   }
