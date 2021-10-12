static char sccsid[] = "src/bos/diag/tu/eth/ex_cmd.c, tu_eth, bos411, 9428A410j 6/19/91 15:00:09";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: execute_cmd
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

Function(s) Execute Command

Module Name :  ex_cmd.c
SCCS ID     :  1.10

Current Date:  6/13/91, 13:11:18
Newest Delta:  2/27/90, 14:03:57

*****************************************************************************/
#include <stdio.h>
#include <time.h>
#include <errno.h>

#include "ethtst.h"	/* note that this also includes hxihtx.h */

#ifdef debugg
extern void detrace();
#endif

int execute_cmd (fdes, cmd_val, tucb_ptr)
   int fdes;
   unsigned char cmd_val;
   TUTYPE *tucb_ptr;
   {
	int rc;
	unsigned long status;
	unsigned short val;
	unsigned long c_time, t_time;
	struct htx_data *htx_sp;

	extern unsigned short swap();
	extern int hio_cmd_wr();
	extern int smem_rd();
	extern int cmd_recd();

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->eth_htx_s.htx_sp;


	/*
	 * write a "execute cmd" byte to command register to
	 * execute the command sitting in the execute mailbox.
	 */
	if (rc = hio_cmd_wr(fdes, cmd_val, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(1,"execute_cmd:  hio_cmd_wr failed!\n");
#endif
		return(EXCMD_WR_ERR);
	   }
	
	/*
	 * Check status register to see that adapter
	 * has read command from the Command Register
	 */
	if (rc = cmd_recd(fdes, tucb_ptr))
	   {
		return(rc);
	   }
	sleep(1);
	
	/*
	 * check if command completed.
	 */
	val = 0;
	if (rc = smem_rd(fdes,
		(unsigned int) tucb_ptr->eth_htx_s.exec_mbox_offset,
		2, (unsigned char *) &val, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(1,"execute_cmd:  smem_rd failed on 1st check\n");
#endif
		return(EXMB_RD_ERR);
	   }
	val = swap(val);
	/*
	 * command completed?
	 */
	if ((val & 0x8000) == 0x8000)
	   {
		/*
		 * command error?
		 */
		if ((val & 0x2000) == 0x2000)
		   {
			if (htx_sp != NULL)
				(htx_sp->bad_others)++;
#ifdef debugg
			detrace(1,"execute_cmd:  cmd completed, but err\n");
#endif
			return(EXMB_ERR_ERR);
		   }

		return(0);
	   }

	t_time = time((long *) 0) + 4;
	while ((c_time = time((long *) 0)) < t_time)
	   {
		if (rc = smem_rd(fdes,
			(unsigned int) tucb_ptr->eth_htx_s.exec_mbox_offset,
			2, (unsigned char *) &val, &status, tucb_ptr))
		   {
#ifdef debugg
			detrace(1,"execute_cmd:  smem_rd failed in loop\n");
#endif
			return(EXMB_RD_ERR);
		   }
		val = swap(val);
		/*
		 * command completed?
		 */
		if ((val & 0x8000) == 0x8000)
		   {
			/*
			 * command error?
			 */
			if ((val & 0x2000) == 0x2000)
			   {
				if (htx_sp != NULL)
					(htx_sp->bad_others)++;
#ifdef debugg
				detrace(1,"execute_cmd:  cmd completed, but err\n");
#endif
				return(EXMB_ERR_ERR);
			   }

			return(0);
		   }
	   }
	
	/*
	 * command still busy?
	 */
	if ((val & 0x4000) == 0x4000)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
#ifdef debugg
		detrace(1,"execute_cmd:  cmd BUSY\n");
#endif
		return(EXMB_BSY_ERR);
	   }
	
#ifdef debugg
		detrace(1,"execute_cmd:  who knows - at end with NO status - timeout\n");
#endif

	if (htx_sp != NULL)
		(htx_sp->bad_others)++;
	return(EXCMD_TIME_ERR);
   }
