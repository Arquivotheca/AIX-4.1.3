static char sccsid[] = "src/bos/diag/tu/eth/tx_stat.c, tu_eth, bos411, 9428A410j 6/19/91 14:56:15";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: tx_complete_status
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

Function(s) Transmit Complete Status

Module Name :  tx_stat.c
SCCS ID     :  1.6

Current Date:  6/13/91, 13:11:29
Newest Delta:  1/19/90, 16:31:46

*****************************************************************************/
#include <stdio.h>
#include <errno.h>

#include "ethtst.h"	/* note that this also includes hxihtx.h */

#ifdef debugg
extern void detrace();
#endif

/*****************************************************************************

tx_complete_status

*****************************************************************************/

int tx_complete_status (fdes, tx_bd_sp, tucb_ptr)
   int fdes;
   struct buffer_descriptor *tx_bd_sp;
   TUTYPE *tucb_ptr;
   {
	int rc;
	unsigned short sval;
	unsigned long c_time, t_time;
	unsigned long status;

	extern unsigned short swap();
	extern int smem_rd();

	/*
	 * Check out the Complete C bit in the Tx buffer descriptor
	 *
	 */
	if (rc = smem_rd(fdes, (unsigned int) tx_bd_sp->a_ctrl_status, 2,
		(unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
    detrace(0,"Checking C bit Tx Buffer at %x\n",tx_bd_sp->a_ctrl_status);
#endif
		return(TXBD_ST_RD_ERR);
	   }
	sval = swap(sval);

	t_time = time((long *)0) + 3;
	while ( (!(sval & 0x0080)) &&
		( (c_time=time((long *)0)) < t_time ) )
	   {
#ifdef debugg
detrace(0,"#2 Checking C bit Tx Buffer sval = %x after swap\n",
	sval);
#endif
		if (rc = smem_rd(fdes,(unsigned int) tx_bd_sp->a_ctrl_status, 2,
			(unsigned char *) &sval, &status, tucb_ptr))
		   {
#ifdef debugg
	detrace(0,"Wrap Failed -- Reading buf_des1 #2 sval\n",sval);
#endif
		return(TXBD_ST_RD_ERR);
		   }
		sval = swap(sval);
#ifdef debugg
	detrace(0,"#3 Checking C bit Tx Buffer sval = %x after swap\n",
		sval);
#endif
	   } /* endwhile */

	if (!(sval & 0x0080))      /* Check for timeout */
	   {
#ifdef debugg
detrace(0,"Wrap Failed -- Timeout in checking for C bit in the Tx buffer\n");
#endif
		return(TXBD_TIME_ERR);
	   }

       /*
	* Check out the OK bit in the Tx buffer descriptor
	*/
	if (!(sval & 0x0040))
	   {
#ifdef debugg
detrace(0,"Wrap failed -- The OK bit was not set in the Tx buffer\n");
#endif
		return(TXBD_NOK_ERR);
	   }
	return(0);
   }
