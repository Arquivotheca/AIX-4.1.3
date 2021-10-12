static char sccsid[] = "src/bos/diag/tu/eth/s_loop.c, tu_eth, bos411, 9428A410j 6/19/91 14:56:46";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: set_loopback
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

Function(s) Set Loopback

Module Name :  s_loop.c
SCCS ID     :  1.6

Current Date:  6/13/91, 13:11:23
Newest Delta:  2/27/90, 14:13:29

*****************************************************************************/
#include <stdio.h>
#include <errno.h>

#include "ethtst.h"	/* note that this also includes hxihtx.h */

#ifdef debugg
extern void detrace();
#endif

int set_loopback (fdes, parm0, tucb_ptr)
   int fdes;
   unsigned short parm0;
   TUTYPE *tucb_ptr;
   {
	int rc;
	short unsigned val;
	unsigned long status;

	extern unsigned short swap();
	extern int smem_wr();
	extern int execute_cmd();

	/*
	 * write "set loopback" command
	 * to the command word of the execute mailbox.
	 */
	val = 0x0009;
	val = swap(val);
	if (rc = smem_wr(fdes,
		(unsigned int) tucb_ptr->eth_htx_s.exec_mbox_offset,
		2, (unsigned char *) &val, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"set_loopback:  smem_wr failed!\n");
#endif
		return(LOOP_C_ERR);
	   }
	/*
	 * write only parameter (looptype - INTERNAL or EXTERNAL)
	 * for parm0 in the execute mailbox.
	 */
#ifdef debugg
	detrace(0,"set_loopback:  value is 0x%04x\n", parm0);
#endif
	parm0 = swap(parm0);
#ifdef debugg
	detrace(1,"set_loopback:  filter value to write (after swap) is 0x%04x\n", parm0);
#endif
	if (rc = smem_wr(fdes,
		(unsigned int) tucb_ptr->eth_htx_s.exec_mbox_offset + 2,
		2, (unsigned char *) &parm0, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"set_loopback:  smem_wr failed!\n");
#endif
		return(LOOP_P_ERR);
	   }

	if (rc = execute_cmd(fdes, 0x40, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"set_loopback:  execute_cmd failed!\n");
#endif
		if (rc == RCMD_NOT_ERR)
			return(LOOP_NOT_ERR);
		return(LOOP_EX_ERR);
	   }

	return(0);
   }
