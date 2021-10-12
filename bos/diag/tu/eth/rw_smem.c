static char sccsid[] = "src/bos/diag/tu/eth/rw_smem.c, tu_eth, bos411, 9428A410j 6/19/91 14:59:46";
/*
 * COMPONENT_NAME: (TU_ETH) Ethernet Test Unit
 *
 * FUNCTIONS: smem_rd, smem_wr
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

Function(s) Read/Write Shared Memory

Module Name :  rw_smem.c
SCCS ID     :  1.7

Current Date:  12/13/90, 07:47:34
Newest Delta:  1/19/90, 16:26:50

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

smem_rd

*****************************************************************************/

int smem_rd (fdes, ram_offset, length, buffer_ptr, status, tucb_ptr)
   int fdes;
   unsigned int ram_offset, length;
   unsigned char *buffer_ptr;
   unsigned long *status;
   TUTYPE *tucb_ptr;
   {
	int rc;
	ccc_mem_acc_t ram;
	struct htx_data *htx_sp;

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->eth_htx_s.htx_sp;

	ram.opcode   =  CCC_READ_OP;
	ram.ram_offset  =  ram_offset;
	ram.length   =  length;
	ram.buffer   =  buffer_ptr;

	*status = 0;
	rc = ioctl(fdes,CCC_MEM_ACC,&ram);
	*status = ram.status;

	if (rc)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(MEM_RD_ERR);
	   }

	if (htx_sp != NULL)
		(htx_sp->good_others)++;
	return(0);
   }

/*****************************************************************************

smem_wr

*****************************************************************************/

int smem_wr (fdes, ram_offset, length, buffer_ptr, status, tucb_ptr)
   int fdes;		     /* Input   */
   unsigned int    ram_offset,length;      /* Inputs  */
   unsigned char   *buffer_ptr;	    /* Input   */
   unsigned long   *status;		/* Output  */
   TUTYPE *tucb_ptr;
   {
	int rc;
	ccc_mem_acc_t  ram;
	struct htx_data *htx_sp;

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->eth_htx_s.htx_sp;

	ram.opcode   =  CCC_WRITE_OP;
	ram.ram_offset   =  ram_offset;
	ram.length   =  length;
	ram.buffer   =  buffer_ptr;

	*status = 0;
	rc = ioctl(fdes,CCC_MEM_ACC,&ram);
	*status = ram.status;

	if (rc)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(MEM_WR_ERR);
	   }

	if (htx_sp != NULL)
		(htx_sp->good_others)++;
	return(0);	     /* If write operation is successful, return 0 */
   }  /* End smem_wr */
