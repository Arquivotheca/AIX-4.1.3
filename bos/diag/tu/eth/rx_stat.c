static char sccsid[] = "src/bos/diag/tu/eth/rx_stat.c, tu_eth, bos411, 9428A410j 6/19/91 15:00:27";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: rx_complete_status
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
Function(s) Receive Completion Status

Module Name :  rx_stat.c
SCCS ID     :  1.9

Current Date:  6/13/91, 13:11:22
Newest Delta:  2/19/91, 13:07:38

*****************************************************************************/
#include <stdio.h>
#include <errno.h>

#include "ethtst.h"	/* note that this also includes hxihtx.h */

#ifdef debugg
extern void detrace();
#endif

/*****************************************************************************
  flip  -  This routine swaps the Hi and Low bytes
*****************************************************************************/
short flip (num)
   short num;
   {
	num = ((num << 8) & 0xFF00) | ((num >> 8) & 0x00FF);
	return(num);
   }

/*****************************************************************************

  lswitch  -  This routine swaps the Hi and Low words in a 32 bit value.

*****************************************************************************/
unsigned long lswitch (num)
   unsigned long num;
   {
	unsigned short snum1, snum2;
	unsigned long lnum1, lnum2;

	num = ((num << 16) & 0xFFFF0000) | ((num >> 16) & 0x0000FFFF);
	snum1 = (unsigned short) ((num >> 16) & 0x0000FFFF);
	snum1 = flip(snum1);
	lnum1 = (unsigned long) snum1;
	snum2 = (unsigned short) (num & 0x0000FFFF);
	snum2 = flip(snum2);
	lnum2 = (unsigned long) snum2;
	num = ((lnum1 << 16) & 0xFFFF0000) | (lnum2 & 0x0000FFFF);

	return(num);
   }

/*****************************************************************************

rx_complete_status

*****************************************************************************/
int rx_complete_status (fdes, rx_bd_sp, tucb_ptr)
   int fdes;
   struct buffer_descriptor *rx_bd_sp;
   TUTYPE *tucb_ptr;
   {
	int rc, save_rc, soff;
	unsigned short sval;
	unsigned long c_time, t_time, prcv, pupl, xfto;
	unsigned long status;
	struct htx_data *htx_sp;

	extern unsigned short swap();
	extern int smem_rd();
	htx_sp = tucb_ptr->eth_htx_s.htx_sp;	

#ifdef debugg
	detrace(0,"rx_stat:  Read rx bd stat at 0x%02x\n",
			rx_bd_sp->a_ctrl_status);
	detrace(0,"rx_stat:  let's read the val of rx mb ptr\n");
	if (rc = smem_rd(fdes,
		(unsigned int) tucb_ptr->eth_htx_s.config_table[3] + 2,
		2, (unsigned char *) &sval, &status, tucb_ptr))
	   {
		detrace(1,"rx_stat:              FAILED\n");
	   }
	detrace(0,"rx_stat:  sval EXACTLY read is 0x%02x\n", sval);
	sval = swap(sval);
	detrace(1,"rx_stat:  after swap, list_ptr pts. to 0x%02x\n", sval);
#endif
       /*
	* Wait for the Receive buffer to complete receiving the packet
	*/
	if (rc = smem_rd(fdes,(unsigned int) rx_bd_sp->a_ctrl_status, 2,
		(unsigned char *) &sval, &status, tucb_ptr))
	   {
#ifdef debugg
	detrace(0,
	"rx_stat:  Checking for the C bit in the Rx Buffer at %x\n",
		rx_bd_sp->a_ctrl_status);
#endif
		return(RXBD_ST_RD_ERR);
	   }
	sval = swap(sval);

#ifdef debugg
	detrace(0,"rx_stat:  Read 0x%02x from rx bd stat\n",
		sval);
	detrace(0,"          Gonna start and'ing with 0x0080 for compare\n");
#endif
	t_time = time((long *)0) + 4;
	while ( (!(sval & 0x0080)) &&
		( (c_time=time((long *)0)) < t_time ) )
	   {
#ifdef debugg
detrace(0,"rx_stat:  #2 Checking C bit Rx Buffer 0x%x sval = 0x%x after swap\n",
	rx_bd_sp->a_ctrl_status, sval);
#endif
		if (rc = smem_rd(fdes,(unsigned int) rx_bd_sp->a_ctrl_status, 2,
			(unsigned char *) &sval, &status, tucb_ptr))
		   {
#ifdef debugg
	detrace(0,"rx_stat:  Trying to read rx bd stat\n");
#endif
			return(RXBD_ST_RD_ERR);
		   }
		sval = swap(sval);
	   } /* endwhile */

	if (!(sval & 0x0080))      /* Check for timeout */
	{
	save_rc = rc;
	soff = (unsigned int) tucb_ptr->eth_htx_s.config_table[6];
        smem_rd(fdes,soff+0x38,4,(unsigned char *)&prcv,&status,tucb_ptr);
        smem_rd(fdes,soff+0x3c,4,(unsigned char *)&pupl,&status,tucb_ptr);
        smem_rd(fdes,soff+0x44,4,(unsigned char *)&xfto,&status,tucb_ptr);

		prcv = lswitch(prcv);
		pupl = lswitch(pupl);
		xfto = lswitch(xfto);

	if (((prcv - pupl) == 1) && (xfto > 0))
	   {
#ifdef debugg
		htx_err(htx_sp,0,7,"No rcvd = 0x%08x\n",prcv);
		htx_err(htx_sp,0,7,"No upld = 0x%08x\n",pupl);
		htx_err(htx_sp,0,7,"No TO = 0x%08x\n",xfto);
#endif
		return(0x8888);
	   }

#ifdef debugg
detrace(1,"\nrx_complete_status:  gonna die now.............\n");
#endif
		return(RXBD_TIME_ERR);
	   }
	if (!(sval & 0x0040))	/* check for okay bit */
	   {
#ifdef debugg
detrace(1,"\nrx_complete_status:  okay bit NOT set.............\n");
#endif
		return(RXBD_NOK_ERR);
	   }
	return(0);
   }
