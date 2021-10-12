static char sccsid[] = "src/bos/diag/tu/eth/rw_hio.c, tu_eth, bos411, 9428A410j 6/19/91 14:55:35";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: hio_wr, hio_rd
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

Function(s) Read/Write HIO Register

Module Name :  rw_hio.c
SCCS ID     :  1.8

Current Date:  6/13/91, 13:11:20
Newest Delta:  1/19/90, 16:26:02

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

hio_wr

*****************************************************************************/

int hio_wr (fdes, hio_reg_offset, hio_val, status, tucb_ptr)
   int fdes;
   unsigned char hio_reg_offset, hio_val;
   unsigned long *status;
   TUTYPE *tucb_ptr;
   {
	int rc;
	ccc_reg_acc_t hio;
	struct htx_data *htx_sp;

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->eth_htx_s.htx_sp;

	hio.opcode  = CCC_WRITE_OP;
	hio.io_reg  = hio_reg_offset;
	hio.io_val  = hio_val;

	*status = 0;
	rc = ioctl(fdes,CCC_REG_ACC,&hio);
	*status  = hio.status;

#ifdef debugg
       detrace(0,"\nhio_wr");
       detrace(0,"\n hio.io_val_o is %x ",hio.io_val_o);
       detrace(0,"\n hio.io_val is %x ",hio.io_val);
       detrace(0,"\n hio.io_reg is %x ",hio.io_reg);
       detrace(0,"\n hio.status is %x ",hio.status);

#endif     

	if (rc)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(1);
	   }

	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	return(0);
   }

/*****************************************************************************

hio_rd

*****************************************************************************/

unsigned char hio_rd (fdes, hio_reg_offset, status, tucb_ptr)
   int fdes;
   unsigned char hio_reg_offset;
   unsigned long *status;
   TUTYPE *tucb_ptr;
   {
	int rc;
	ccc_reg_acc_t hio;
	struct htx_data *htx_sp;

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->eth_htx_s.htx_sp;

	hio.opcode = CCC_READ_OP;
	hio.io_reg = hio_reg_offset;

	*status = 0;
	rc = ioctl(fdes,CCC_REG_ACC,&hio);
	*status  = hio.status;

#ifdef debugg
       detrace(0,"\nhio_rd");
       detrace(0,"\n hio.io_val_o is %x ",hio.io_val_o);
       detrace(0,"\n hio.io_val is %x ",hio.io_val);
       detrace(0,"\n hio.io_reg is %x ",hio.io_reg);
       detrace(0,"\n hio.status is %x ",hio.status);
       detrace(0,"\n hio.io_status is %x ",hio.io_status);

#endif     

	if (rc)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(1);
	   }

	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	return(hio.io_val);
   }
