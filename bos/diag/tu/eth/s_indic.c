static char sccsid[] = "src/bos/diag/tu/eth/s_indic.c, tu_eth, bos411, 9428A410j 6/19/91 14:55:49";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: set_indication_enable
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

Function(s) Set Indication Enable

Module Name :  s_indic.c
SCCS ID     :  1.6

Current Date:  6/13/91, 13:11:22
Newest Delta:  2/27/90, 14:13:13

*****************************************************************************/
#include <stdio.h>
#include <errno.h>

#include "ethtst.h"	/* note that this also includes hxihtx.h */

#ifdef debugg
extern void detrace();
#endif

int set_indication_enable (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	short unsigned sval;
	unsigned long status;

	extern unsigned short swap();
	extern int smem_wr();
	extern int execute_cmd();

	/*
	 * write "set indication enable" command
	 * to the command word of the execute mailbox.
	 */
	sval = 0x0004;
	sval = swap(sval);
	if (rc = smem_wr(fdes,
		(unsigned int) tucb_ptr->eth_htx_s.exec_mbox_offset,
		2, (unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"set_indic:  smem_wr failed!\n");
#endif
		return(IND_C_ERR);
	   }

	/*
	 * write enable parameter 
	 * in the execute mailbox.
	 */
	sval = 0x0001;
	sval = swap(sval);
#ifdef debugg
	detrace(1,"set_indic:  indic value to write (after swap) is 0x%04x\n",
		sval);
#endif
	if (rc = smem_wr(fdes,
		(unsigned int) tucb_ptr->eth_htx_s.exec_mbox_offset + 2,
		2, (unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"set_indic:  smem_wr failed!\n");
#endif
		return(IND_P_ERR);
	   }

	if (rc = execute_cmd(fdes, 0x40, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"set_indic:  execute_cmd failed!\n");
#endif
		if (rc == RCMD_NOT_ERR)
			return(IND_NOT_ERR);
		return(IND_EX_ERR);
	   }

	return(0);
   }
