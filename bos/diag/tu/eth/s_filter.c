static char sccsid[] = "src/bos/diag/tu/eth/s_filter.c, tu_eth, bos411, 9428A410j 6/19/91 15:01:20";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: set_filter
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

Function(s) Set Receive Filter

Module Name :  s_filter.c
SCCS ID     :  1.6

Current Date:  6/13/91, 13:11:22
Newest Delta:  2/27/90, 14:12:55

*****************************************************************************/
#include <stdio.h>
#include <errno.h>

#include "ethtst.h"	/* note that this also includes hxihtx.h */

#ifdef debugg
extern void detrace();
#endif

int set_filter (fdes, parm0, tucb_ptr)
   int fdes;
   unsigned short parm0;
   TUTYPE *tucb_ptr;
   {
	int rc;
	short unsigned sval;
	unsigned long status;

	extern unsigned short swap();
	extern int smem_wr();
	extern int execute_cmd();

	/*
	 * write "set receive filter" command
	 * to the command word of the execute mailbox.
	 */
	sval = 0x0000;
	if (rc = smem_wr(fdes,
		(unsigned int) tucb_ptr->eth_htx_s.exec_mbox_offset,
		2, (unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"set_filter:  smem_wr failed!\n");
#endif
		return(SFIL_C_ERR);
	   }
	/*
	 * write only parameter (receive filter value)
	 * for parm0 in the execute mailbox.
	 */
#ifdef debugg
	detrace(0,"set_filter:  filter value is 0x%04x\n", parm0);
#endif
	parm0 = swap(parm0);
#ifdef debugg
	detrace(1,"set_filter:  filter value to write (after swap) is 0x%04x\n", parm0);
#endif
	if (rc = smem_wr(fdes,
		(unsigned int) tucb_ptr->eth_htx_s.exec_mbox_offset + 2,
		2, (unsigned char *) &parm0, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"set_filter:  smem_wr failed!\n");
#endif
		return(SFIL_P_ERR);
	   }

	if (rc = execute_cmd(fdes, 0x40, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"set_filter:  execute_cmd failed!\n");
#endif
		if (rc == RCMD_NOT_ERR)
			return(SFIL_NOT_ERR);
		return(SFIL_EX_ERR);
	   }

	return(0);
   }
