static char sccsid[] = "src/bos/diag/tu/eth/rw_pos.c, tu_eth, bos411, 9428A410j 6/19/91 14:57:38";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: pos_wr, pos_rd, init_pos
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

Function(s) Read/Write POS Register

Module Name :  rw_pos.c
SCCS ID     :  1.8

Current Date:  12/13/90, 07:47:34
Newest Delta:  1/19/90, 16:26:26

*****************************************************************************/
#include <stdio.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/entuser.h>
#include <errno.h>

#include "ethtst.h"

#ifdef debugg
extern void detrace();
#endif

/*****************************************************************************

pos_wr

*****************************************************************************/

int pos_wr (fdes, pos_reg, pos_val, status, tucb_ptr)
   int fdes;
   unsigned char pos_reg,
		 pos_val;
   unsigned long *status;
   TUTYPE *tucb_ptr;
   {
	int rc;
	ccc_pos_acc_t pos;
	struct htx_data *htx_sp;

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->eth_htx_s.htx_sp;

	pos.opcode   = CCC_WRITE_OP;
	pos.pos_reg  = pos_reg;
	pos.pos_val  = pos_val;

	*status = 0;
	rc = ioctl(fdes,CCC_POS_ACC,&pos);
	*status  = pos.status;

	if (rc)
	   {
#ifdef debugg
  detrace(0,"POS write failed ioctl returned = %x\n pos.pos_reg = %x\n pos.pos_val = %x\n pos.opcode = %x\n status = %x\n fd = %x\n",rc,pos_reg,pos_val,CCC_WRITE_OP,pos.status,fd);
#endif
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
	       return(1);
	   }

	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	return(0);
   }  /* End pos_wr */

/*****************************************************************************

pos_rd

*****************************************************************************/

unsigned char pos_rd (fdes, pos_reg, status, tucb_ptr)
   int fdes;
   unsigned char pos_reg;
   unsigned long *status;
   TUTYPE *tucb_ptr;
   {
	int rc;
	ccc_pos_acc_t pos;
	struct htx_data *htx_sp;

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->eth_htx_s.htx_sp;

	pos.opcode  = CCC_READ_OP;
	pos.pos_reg = pos_reg;
	*status = 0;
	rc = ioctl(fdes,CCC_POS_ACC,&pos);
	*status = pos.status;

	if (rc)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(1);
	   }

	if (htx_sp != NULL)
		(htx_sp->good_others)++;
	return(pos.pos_val);
   }  /* End pos_rd */

/*****************************************************************************

init_pos

Function makes sure that the card is enabled and that POS registers [6] & [7]
are set to zero.

IF successful 
THEN RETURNs 0
ELSE RETURNs error code relating to read/write/compare problem with a POS

*****************************************************************************/

int init_pos (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	unsigned long status;
        unsigned char p0, p1;
	struct htx_data *htx_sp;
	int pos_wr();
	unsigned char pos_rd();

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->eth_htx_s.htx_sp;

        /* 
         *   Read POS2 
         */
        p0 = pos_rd(fdes, 2, &status, tucb_ptr);
	if (status != 0)
	   {
		return(POS2_RD_ERR);
	   }
        p0 = p0 | 0x01; 

	/*
	 * Enable the card by writing to POS 2.
	 */
	if (pos_wr(fdes, 2, p0, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"pos write of POS2 with 01 failed!\n");
#endif
		return(POS2_WR_ERR);
	   }

	/*
	 * Write a 0 to POS 6.
	 */
	if (pos_wr(fdes, 6, 0x00, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"pos write of POS6 with 00 failed!\n");
#endif
		return(POS6_WR_ERR);
	   }
	
	/*
	 * Write a 0 to POS 7.
	 */
	if (pos_wr(fdes, 7, 0x00, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"pos write of POS7 with 00 failed!\n");
#endif
		return(POS7_WR_ERR);
	   }
    return(0);
   }
