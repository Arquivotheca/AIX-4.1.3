static char sccsid[] = "src/bos/diag/tu/eth/cmd_recd.c, tu_eth, bos411, 9428A410j 5/12/92 12:46:27";
/*
 * COMPONENT_NAME: (ETHERTU) Ethernet Test Unit
 *
 * FUNCTIONS: cmd_recd
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990, 1991, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <time.h>
#include "ethtst.h"

#define CRR_MASK	0x40
/*****************************************************************************

cmd_recd

Polls status register to determine when adapter's 80186 has read
the command register.

*****************************************************************************/

int cmd_recd (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	long t_time, c_time;
	unsigned long status;
	int crr_zero;
	unsigned char status_byte;
	extern unsigned char hio_status_rd();

	crr_zero = 0;
	status = 0;
	t_time = time((long *) 0) + 2;
	while ((c_time = time((long *) 0)) < t_time)
	   {
		status_byte = hio_status_rd(fdes, &status, tucb_ptr);
		if (status)
			return(RSTA_RD_ERR);
		if ((status_byte & CRR_MASK) == CRR_MASK)
		   {
			crr_zero = 1;
			break;
		   }
	   }

	/*
	   Check status register one last time after loop to 
	   avoid timeouts due to system delays.
	*/ 

	if (!crr_zero)
	   {  
		status_byte = hio_status_rd(fdes, &status, tucb_ptr);
                if (status)
                        return(RSTA_RD_ERR);
                if ((status_byte & CRR_MASK) == CRR_MASK)
                        crr_zero = 1;
           }


	if (crr_zero)
		return(0);
	return(RCMD_NOT_ERR);
   }
