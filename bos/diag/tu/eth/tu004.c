static char sccsid[] = "src/bos/diag/tu/eth/tu004.c, tu_eth, bos411, 9428A410j 5/12/92 13:06:03";
/*
 * COMPONENT_NAME: (ETHERTU) Ethernet Test Unit
 *
 * FUNCTIONS: io_chk, tu004
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
/*****************************************************************************

Function(s) Test Unit 004 - I/O Register Test

*****************************************************************************/
#include <stdio.h>
#include <errno.h>
#include "ethtst.h"

#define CWR_MASK	0x20

#ifdef debugg
extern void detrace();
#endif

/*****************************************************************************

io_chk

*****************************************************************************/

int io_chk (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	unsigned long c_time, t_time, status;
	unsigned char p0, cval, testval;
	int rc;
	short val;
	struct htx_data *htx_sp;

	extern int hard_reset();
	extern int hio_ctrl_wr();
	extern int smem_wr();
	extern int hio_cmd_wr();
	extern unsigned char hio_cmd_rd();
	extern unsigned char hio_status_rd();
	extern int mktu_rc();
	extern int execute_cmd();

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->eth_htx_s.htx_sp;

	/*
	 * Do a hard_reset
	 */
	if (rc = hard_reset(fdes, tucb_ptr))
	   {
		/*
		 * mask out the top two bytes which
		 * may have error code info since it
		 * is also called as a separate test unit.
		 */
		rc &= 0x0000ffff;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
	   }
	
	/*
	 * Disable the Interrupts to directly access
	 * the command register
	 */
	if (hio_ctrl_wr(fdes,0x00,&status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"Disable interrupts Failed\n");
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RCTL_WR_ERR));
	   }
	
	/*
	 * Execute Command Register Wrap,
	 * Write 7 to Execute Mailbox
	 */
	val = 0x0700;
	if (rc = smem_wr(fdes,tucb_ptr->eth_htx_s.exec_mbox_offset,2,
				&val,&status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"Write Failed to Memory 0x2d4\n");
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, TXMB_WR_ERR));
	   }

	/*
	 * Execute the command (Register "Wrap") in the execute mailbox.
	 */
	if (rc = execute_cmd(fdes, 0x40, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"io_chk:  execute_cmd on reg wrap failed!\n");
#endif
		if (rc == RCMD_NOT_ERR)
			return(mktu_rc(tucb_ptr->header.tu,
					LOG_ERR, REG_NOT_ERR));
		return(mktu_rc(tucb_ptr->header.tu,
					LOG_ERR, REG_WRAP_ERR));
	   }

	/*
	 * Read the Command Register,
	 * make sure it is 0
	 */
	cval = hio_cmd_rd(fdes, &status, tucb_ptr);
	if (status)
	   {
#ifdef debugg
		detrace(0,"Command Register Read Fail #2\n");
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RCMD_RD_ERR));
	   }

	if (cval)
	   {
#ifdef debugg
		detrace(0,"EX command did not confirm\n");
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, REG_WRAP2_ERR));
	   }

	/*
	 * Write a pattern to the command register
	 */
	testval = 0xAA;
	if (hio_cmd_wr(fdes,testval,&status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"Command Register Write Failed #2\n");
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RCMD_WR_ERR));
	   }

	/*
	 * Wait for pattern to appear
	 */
	cval = hio_status_rd(fdes,&status, tucb_ptr);
	if (status != 0)
	   {
#ifdef debugg
		detrace(0,"Status Register Read Failed #3\n");
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RSTA_RD_ERR));
	   }

	/*
	 * WAIT UNTIL Adapter Writes
	 * to CMD REG or FOR 5 Seconds.
	 * i.e. Wait for EX command confirmation
	 */

	t_time = time((long *)0) + 5;
	while ( ((cval & CWR_MASK) == 0x00) &&
		((c_time = time((long *)0)) < t_time) )
	   {
		/*
		 * Read the Status Register
		 */
		cval = hio_status_rd(fdes,&status, tucb_ptr);
		if (status != 0)
		   {
#ifdef debugg
			detrace(0,"Status Register Read Failed #4\n");
#endif
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
					RSTA_RD_ERR));
		   }
	   } /* endwhile */

	/* Read status register twice after the loop since to avoid 
	   timeouts because of system delays. If data is not available 
	   at the status register the go check the status queue.
	*/

	if ((cval & CWR_MASK) == 0x00)
	   {
		cval = hio_status_rd(fdes,&status, tucb_ptr);
		if (status != 0)
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				RSTA_RD_ERR));
	   }

        if ((cval & CWR_MASK) == 0x00)
           {
                cval = hio_status_rd(fdes,&status, tucb_ptr);
                if (status != 0)
                        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                RSTA_RD_ERR));
           }

        if ((cval & CWR_MASK) == 0x00)
           {
                cval = hio_status_rd_q(fdes,&status, tucb_ptr);
                if (status != 0)
                        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                QSTA_RD_ERR));
           }

	if ((cval & CWR_MASK) == 0x00)      /* Check for timeout */
	   {

#ifdef debugg
		detrace(0,"Timeout in Wait until CWR bit set loop #2\n");
#endif
			    return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
					RSTA_TIME_ERR));
	    }

	/*
	 * Read the test pattern
	 * to verify the CMD REG write
	 */
	cval = hio_cmd_rd(fdes,&status, tucb_ptr);
	if (status != 0)
	   {
#ifdef debugg
		detrace(0,"Command Register Read Failed #2\n");
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RCMD_RD_ERR));
	   }

	if (cval != testval)
	   {
#ifdef debugg
		detrace(0,"Command Register Read Failed #2\n");
#endif
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RCMD_CMP_ERR));
	   }

#ifdef debugg
	detrace(0,"Done for now testval = %x cval = %x\n",testval,cval);
#endif
	
	/*
	 * Execute Hard Reset to stop the Command Register "Wrap"
	 */ 
	if (rc = hard_reset(fdes, tucb_ptr))
	   {
		/*
		 * mask out the top two bytes which
		 * may have error code info since it
		 * is also called as a separate test unit.
		 */
		rc &= 0x0000ffff;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
	   }

	return(0);
   }

/*****************************************************************************

tu004

*****************************************************************************/

int tu004 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	return(io_chk(fdes, tucb_ptr));
   }
