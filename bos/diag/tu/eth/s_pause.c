static char sccsid[] = "src/bos/diag/tu/eth/s_pause.c, tu_eth, bos411, 9428A410j 6/19/91 14:58:52";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: set_pause
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

Function(s) Set PAUSE command

Module Name :  s_pause.c
SCCS ID     :  1.3

Current Date:  6/13/91, 13:11:23
Newest Delta:  2/27/90, 14:14:16

*****************************************************************************/
#include <stdio.h>
#include <errno.h>

#include "ethtst.h"	/* note that this also includes hxihtx.h */

#ifdef debugg
extern void detrace();
#endif

int set_pause (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	short unsigned sval;
	unsigned long status;

	extern unsigned short swap();
	extern int smem_wr();
	extern int smem_rd();
	extern int execute_cmd();

	/*
	 * write only parameter (pause value - all zeroes)
	 * for parm 0 in the execute mailbox.
	 */
	sval = 0x0000;
#ifdef debugg
	detrace(0,"set_pause:  pause value is 0x%04x\n", sval);
#endif
#ifdef debugg
	detrace(1,"set_pause:  value to write (after swap) is 0x%04x\n", sval);
#endif
	if (rc = smem_wr(fdes,
		(unsigned int) tucb_ptr->eth_htx_s.exec_mbox_offset + 2,
		2, (unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"set_pause:  smem_wr failed!\n");
#endif
		return(SPAU_P_ERR);
	   }

	/*
	 * write "pause command"
	 * to the command word of the execute mailbox.
	 */
	sval = 0x00c3;
	sval = swap(sval);
	if (rc = smem_wr(fdes,
		(unsigned int) tucb_ptr->eth_htx_s.exec_mbox_offset,
		2, (unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"set_pause:  smem_wr failed!\n");
#endif
		return(SPAU_C_ERR);
	   }

	if (rc = execute_cmd(fdes, 0x40, tucb_ptr))
	   {
		if (rc == RCMD_NOT_ERR)
			return(SPAU_NOT_ERR);
		return(SPAU_EX_ERR);
	   }
	
	/*
	 * sleep two seconds to allow 80186 enough time
	 * to write 0xffff to 2nd word of execute mailbox
	 * which is telling us that he is pausing...
	 */
	sleep(2);
	if (rc = smem_rd(fdes,
		(unsigned int) tucb_ptr->eth_htx_s.exec_mbox_offset + 2,
		2, (unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"set_pause:  Read of parm0 after execute failed!\n");
#endif
		return(SPAU_RD_ERR);
	   }
	sval = swap(sval);
	if (sval != 0xffff)
		return(SPAU_FF_ERR);

	return(0);
   }
