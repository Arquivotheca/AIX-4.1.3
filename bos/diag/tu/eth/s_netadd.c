static char sccsid[] = "src/bos/diag/tu/eth/s_netadd.c, tu_eth, bos411, 9428A410j 6/19/91 14:57:50";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: set_netadd
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

Function(s) Set Network Address

Module Name :  s_netadd.c
SCCS ID     :  1.6

Current Date:  6/13/91, 13:11:23
Newest Delta:  2/27/90, 14:13:47

*****************************************************************************/
#include <stdio.h>
#include <errno.h>

#include "ethtst.h"	/* note that this also includes hxihtx.h */

#ifdef debugg
extern void detrace();
#endif

int set_netadd (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	short unsigned sval;
	unsigned char *ucp;
	unsigned long status;

	extern unsigned short swap();
	extern int smem_wr();
	extern int execute_cmd();

	/*
	 * write "set network address" command
	 * to the command word of the execute mailbox.
	 */
	sval = 0x0001;
	sval = swap(sval);
	if (rc = smem_wr(fdes,
		(unsigned int) tucb_ptr->eth_htx_s.exec_mbox_offset,
		2, (unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"set_netadd:  smem_wr failed!\n");
#endif
		return(SNET_C_ERR);
	   }

	ucp = tucb_ptr->eth_htx_s.net_addr;
	/*
	 * write 2nd and 1st (LSB) bytes of network address
	 * for parm 0 in the execute mailbox.
	 */
	sval = (*(ucp + 0) << 8) | *(ucp + 1);
/*
	sval = swap(sval);
*/
#ifdef debugg
	detrace(0,"set_netadd:  parm0 (offset 2) is %04x\n", sval);
#endif
	if (rc = smem_wr(fdes,
		(unsigned int) tucb_ptr->eth_htx_s.exec_mbox_offset + 2,
		2, (unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"set_netadd:  smem_wr failed!\n");
#endif
		return(SNET_P_ERR);
	   }

	/*
	 * write 4th and 3rd (LSB) bytes of network address
	 * for parm 1 in the execute mailbox.
	 */
	sval = (*(ucp + 2) << 8) | *(ucp + 3);
/*
	sval = swap(sval);
*/
#ifdef debugg
	detrace(0,"set_netadd:  parm1 (offset 4) is %04x\n", sval);
#endif
	if (rc = smem_wr(fdes,
		(unsigned int) tucb_ptr->eth_htx_s.exec_mbox_offset + 4,
		2, (unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"set_netadd:  smem_wr failed!\n");
#endif
		return(SNET_P_ERR);
	   }

	/*
	 * write 6th and 5th (MSB) bytes of network address
	 * for parm 1 in the execute mailbox.
	 */
	sval = (*(ucp + 4) << 8) | *(ucp + 5);
/*
	sval = swap(sval);
*/
#ifdef debugg
	detrace(0,"set_netadd:  parm2 (offset 6) is %04x\n", sval);
#endif
	if (rc = smem_wr(fdes,
		(unsigned int) tucb_ptr->eth_htx_s.exec_mbox_offset + 6,
		2, (unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"set_netadd:  smem_wr failed!\n");
#endif
		return(SNET_P_ERR);
	   }

#ifdef debugg
	detrace(1,"set_netadd:  ready execute mbox\n");
#endif
	if (rc = execute_cmd(fdes, 0x40, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"set_netadd:  execute_cmd failed!\n");
#endif
		if (rc == RCMD_NOT_ERR)
			return(SNET_NOT_ERR);
		return(SNET_EX_ERR);
	   }
	return(0);
   }
