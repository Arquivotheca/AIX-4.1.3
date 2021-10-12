static char sccsid[] = "src/bos/diag/tu/eth/r_stat.c, tu_eth, bos411, 9428A410j 5/12/92 12:53:50";
/*
 * COMPONENT_NAME: (ETHERTU) Ethernet Test Unit
 *
 * FUNCTIONS: hio_status_rd, hio_status_rd_q, hio_status_wr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/********************************************************************

Function(s) Read HIO Status Register

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

hio_status_rd

*****************************************************************************/

unsigned char hio_status_rd (fdes, status, tucb_ptr)
   int fdes;
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
	hio.io_reg  = 2; 

	*status = 0;
	rc = ioctl(fdes,CCC_REG_ACC,&hio);
	*status  = hio.status;

#ifdef debugg 

	detrace(0,"\nhio_status_rd");
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

/*****************************************************************************

hio_status_rd_q

*****************************************************************************/

unsigned char hio_status_rd_q (fdes, status, tucb_ptr)
   int fdes;
   unsigned long *status;
   TUTYPE *tucb_ptr;
   {
	unsigned char rc;
	ccc_reg_acc_t hio;
	struct htx_data *htx_sp;

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->eth_htx_s.htx_sp;

	hio.opcode = CCC_READ_Q_OP;
	hio.io_reg  = 2; 

	*status = 0;
	rc = ioctl(fdes,CCC_REG_ACC,&hio);
	*status  = hio.status;

#ifdef debugg  

	detrace(0,"\nhio_status_rd_q");
	detrace(0,"\n hio.io_val_o is %x rc = %x",hio.io_val_o,rc);
	detrace(0,"\n hio.io_val is %x ",hio.io_val);
	detrace(0,"\n hio.io_reg is %x ",hio.io_reg);
	detrace(0,"\n hio.status is %x ",hio.status);
	detrace(0,"\n hio.io_status is %x ",hio.io_status);

#endif   

	if (rc)
		return(1);

	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	return(hio.io_val);
    }
/*****************************************************************************

hio_status_wr

******************************************************************************/

int hio_status_wr (fdes, hio_val, status, tucb_ptr)
   int fdes;
   unsigned char hio_val;
   unsigned long *status;
   TUTYPE *tucb_ptr;
   {
	int rc;
	ccc_reg_acc_t  hio;
	struct htx_data *htx_sp;

	/*
	 * set up pointer to HTX data structure to
	 * increment counters in case TU was invoked by
	 * hardware exerciser.
	 */

	htx_sp = tucb_ptr->eth_htx_s.htx_sp;

	hio.opcode = CCC_WRITE_OP;
	hio.io_reg = 2;           /* status register location */
	hio.io_val = hio_val;

	*status = hio.status;
	rc = ioctl(fdes, CCC_REG_ACC, &hio);
	*status = hio.status;

#ifdef	debugg 

	detrace(0,"\n hio_status_wr");
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
