static char sccsid[] = "src/bos/diag/tu/tok/tu001.c, tu_tok, bos411, 9428A410j 6/9/94 11:21:01";
/*
 * COMPONENT_NAME: (TOKENTU) Token Test Unit
 *
 * FUNCTIONS: setup_pos, tu001
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
/*****************************************************************************

Function(s) Test Unit 001 - Start Adapter

Module Name :  16
SCCS ID     :  1.3.1.1

Current Date:  5/6/94, 11:51:22
Newest Delta:  11/18/91, 14:39:43

Before the adapter is actually opened via START, an ioctl is performed
to the device driver to set up open options.  Then, the test unit
will do a start on the adapter (with the previously set open options) and
lookup the network address being used for the card.

As specified by the device driver, the first START provides the
ADAPTER INITIALIZATION command to the adapter.  Upon receipt,
the adapter invokes internal diagnostics to test the Processor,
RAM, ROS, Protocol Handler, SIF, and FEC (as stated in the
SPYGLASS Microcode documentation).

*****************************************************************************/
#include <stdio.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/ioctl.h>
#include <sys/tokuser.h>
#include <string.h>
#include <memory.h>
#include <errno.h>

#include "toktst.h"

#if 0
#define BRING_UP_MASK	0x00000030
#define BRING_UP_ERRMSK	0x00008400
#define INIT_MASK	0x000000C0
#define INIT_ERRMSK	0x00008800
#endif

#define START_ATTEMPTS  150	/* max. times to test for START completion */

#define SLEEP_INTERVAL    2	/*
				 * seconds of sleep interval while waiting
				 * for START completion
				 */
#define START_ASYNC_ATTEMPTS 5   /* max. times to test for ASYNC */ 

#ifdef debugg
extern void detrace();
#endif

/******************************************************************************

setup_pos

Function setups up POS registers to desired settings for
FAIRNESS, STREAMING DATA, and PREEMPT-TIMEOUT.

******************************************************************************/

int setup_pos (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int cc;
	unsigned char pos_val, sav_val;
	struct htx_data *htx_sp;
	extern int read_pos();
	extern int write_pos();
	extern int mktu_rc();

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->token_s.htx_sp;

	/*
	 * Read current setting of POS 4
	 */
	if (cc = read_pos(fdes, 4, &pos_val, tucb_ptr))
		return(cc);

	/*
	 * Set fairness accordingly...
	 */
	if ((strcmp(tucb_ptr->token_s.fairness,"ON") == 0) ||
	    (strcmp(tucb_ptr->token_s.fairness,"on") == 0))
		pos_val &= 0xef;
	else
		pos_val |= 0x10;
	
	/*
	 * Set preempt timeout accordingly...
	 */
	if ((strcmp(tucb_ptr->token_s.preempt,"IMMED") == 0) ||
	    (strcmp(tucb_ptr->token_s.preempt,"immed") == 0))
		pos_val &= 0xdf;
	else
		pos_val |= 0x20;

	/*
	 * save off value to write to POS
	 */
	sav_val = pos_val;
	if (cc = write_pos(fdes, 4, pos_val, tucb_ptr))
		return(cc);
	
	/*
	 * Now read it back to make everything got set right.
	 */
	pos_val = 0;
	if (cc = read_pos(fdes, 4, &pos_val, tucb_ptr))
		return(cc);
	
	if (pos_val != sav_val)
	   {
		if (htx_sp != NULL)
			htx_sp->bad_others++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS4_CMP_ERR));
	   }

	/*
	 * Read current setting of POS 5
	 */
	if (cc = read_pos(fdes, 5, &pos_val, tucb_ptr))
		return(cc);

	/*
	 * Set streaming_data accordingly...
	 */
	if ((strcmp(tucb_ptr->token_s.streaming,"OFF") == 0) ||
	    (strcmp(tucb_ptr->token_s.streaming,"off") == 0))
		pos_val &= 0xdf;
	else
		pos_val |= 0x20;

	
	/*
	 * save off value to write to POS
	 */
	sav_val = pos_val;
	if (cc = write_pos(fdes, 5, pos_val, tucb_ptr))
		return(cc);

	/*
	 * Check that POS 5 setup correctly 
	 */
	pos_val = 0;
	if (cc = read_pos(fdes, 5, &pos_val, tucb_ptr))
		return(cc);
	
	if (pos_val != sav_val)
	   {
		if (htx_sp != NULL)
			htx_sp->bad_others++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS5_CMP_ERR));
	   }

	return(0);
   }

/******************************************************************************

tu001

******************************************************************************/

int tu001 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	struct session_blk start_s;
	struct status_block status_s;
	struct SET_OPEN_OPTS tr_opts_s;

	struct devinfo devinfo_s;
	unsigned char net_add[NETADD_LEN];
	register int i;
	int cc, uncode, save_errno;

	struct htx_data *htx_sp;
	extern int errno;
	extern int tr_stat();
	extern int mktu_rc();
	extern int tr_uncode();
	extern int qvpd();


#ifdef debugg
	detrace(0,"\n\ntu001:  Begins...\n");
#endif
	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->token_s.htx_sp;

	/*
	 * perform ioctl on vital product data and grab network address.
	 */
	cc = qvpd(fdes, tucb_ptr, net_add);
	if (cc)
	   {
#ifdef debugg
	detrace(0,"tu001:  query to VPD failed.\n");
#endif
		/*
		 * note that error value returned from qvpd()
		 * has already been masked using mktu_rc, so
		 * don't need to do so, here.
		 */
		return(cc);
	   }
#ifdef debugg
	detrace(0,"tu001:  vpd worked....net add is:  ");
	for (i = 0; i < NETADD_LEN; i++)
		detrace(0,"%02X", *(net_add + i));
	detrace(0,"\n\n");
#endif


	/*
	 * set up the option options parms BEFORE 
	 * doing the START to open the adapter.
	 */
	

	/*
	 * If running under HTX, then can
	 * setup POS registers for fairness, streaming data,
	 * and preempt-timeout.
	 */
	if (htx_sp != NULL)
	   {
		if (cc = setup_pos(fdes, tucb_ptr))
		   {
			return(cc);
		   }
	   }
	
	/*
	 * options 0xb500 specify (note bit 0 is MOST significant bit):
	 *
	 *  bit 0 - negate ring insertion causing data to WRAP
	 *      1 - do NOT disable HARD ERROR interrupts
	 *      2 - disable SOFT ERROR interrupts
	 *      3 - pass MAC frames as normal receive data
	 *      4 - no attention MAC frames are passed to system
	 *      5 - pad ROUTING field - NECESSARY for device driver!!!
	 *      6 - do NOT wait for rec'v completion before
	 *              initiating transfer of frame to system
	 *      7 - participate in Monitor Contention if opted
	 *      8 - do NOT pass Beacon MAC frames
	 *      9 - enable DMA Timeout 10 sec. timer
	 *
	 *  10 - 15  UNUSED
	 */
	tr_opts_s.options = 0xb500;
	tr_opts_s.buf_size = 0x0400;
	tr_opts_s.xmit_buf_min_cnt = 0x04;
	tr_opts_s.xmit_buf_max_cnt = 0x08;
	memcpy((char *) &(tr_opts_s.i_addr1), net_add, 2);
	memcpy((char *) &(tr_opts_s.i_addr2), net_add + 2, 2);
	memcpy((char *) &(tr_opts_s.i_addr3), net_add + 4, 2);
/*
	tr_opts_s.i_addr1 = 0x00F7;
	tr_opts_s.i_addr2 = 0x0088;
	tr_opts_s.i_addr3 = 0x0088;
*/

#ifdef debugg
	detrace(0,"tu001:  Before ioctl to OPEN_PARMS\n");
#endif
	cc = ioctl(fdes, TOK_SET_OPEN_PARMS, &tr_opts_s);
	save_errno = errno;
#ifdef debugg
	detrace(0,"tu001:  After ioctl to OPEN_PARMS\n");
	if (cc)
	   {
		if (save_errno == EIO)
		   {
		detrace(0,"tu001:  OPEN_PARMS failed!!!  errno is %d\n",
			save_errno);
		detrace(1,"tu001:  status ret'nd was hex %0x\n",
			tr_opts_s.status);
		   }
		else
		   detrace(1,"tu001:  OPEN_PARMS failed!!!  errno is %d\n",
			save_errno);
	   }
#endif
	if (cc)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		
		if (save_errno == EIO)
		   {
			if (tr_opts_s.status == TOK_NOT_DIAG_MODE)
				return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
					NOT_DIAG_ERR));

			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				OPEN_OPT_ERR));
		   }
		else
			return(mktu_rc(tucb_ptr->header.tu, SYS_ERR,
				save_errno));
	   }

	/*
	 * set up the netid for the session with a default
	 * as specified by TU_NET_ID (defined in toktst.h) and
	 * then start device.
	 */
	start_s.netid = TU_NET_ID;
	start_s.length = 1;
	cc = ioctl(fdes, CIO_START, &start_s);
#ifdef debugg
	detrace(0,"tu001:  AFTER ioctl CIO_START.  cc = %d\n", cc);
	if (cc)
		detrace(1,"tu001:  errno is %d\n", errno);
#endif
	if (cc)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		

		if (errno = EIO)
		   {
#if 0 /* NETID COMMENTED OUT!!!  REMEMBER TO FIX */
			if ((start_s.status == CIO_NETID_FULL) ||
			    (start_s.status == CIO_NETID_INV)  ||
			    (start_s.status == CIO_NETID_DUP))
				return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
					NETID_ERR));
			else
#endif
				return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
					TOK_ST1_ERR));
		   }
		return(mktu_rc(tucb_ptr->header.tu, SYS_ERR, errno));
	   }

#ifdef debugg
	detrace(0,"tu001:  Okay, let's poll for a START_DONE...\n");
#endif
	/*
	 * perform loop (inside tr_stat) on get status 'til receive
	 * notification of completion of start.  If a non-zero is
	 * returned then got an ioctl error or we never saw a
	 * CIO_START_DONE so report what I did get instead.
	 */
	if (cc = tr_stat(fdes, &status_s, htx_sp, CIO_START_DONE,
		START_ATTEMPTS, SLEEP_INTERVAL))
	   {
#ifdef debugg
		detrace(0,"tu001:  NEVER got START_DONE....\n");
#endif
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;

		if (cc < 0)
			/*
			 * the tr_stat failed on its own ioctl
			 * so make the return code with the errno
			 * produced from within tr_stat that was sent
			 * back inside status_s.code.
			 */

			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				ST3_DONE_ERR));

		else
		   {
			/*
			 * never got a CIO_START_DONE, so lookup
			 * the adapter error in the status structure
			 * to make the return value.
			 */
			uncode = tr_uncode(&status_s);
			if (uncode < 0)
				return(mktu_rc(tucb_ptr->header.tu,
					LOG_ERR, ST1_DONE_ERR));

			return(mktu_rc(tucb_ptr->header.tu, TOK_ERR,
				uncode));
		   }
	   }
	
#ifdef debugg
	detrace(0,"tu001:  We got a START_DONE!!!\n");
#endif
	/*
	 * okay, we did get a CIO_START_DONE, but we need to check
	 * that it was a successful completion.
	 */
	if (status_s.option[0] != CIO_OK)
	   {
#ifdef debugg
		detrace(0,"tu001:  But START didn't ret'n CIO_OK!\n");
#endif
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		
		uncode = tr_uncode(&status_s);
		if (uncode < 0)
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				ST2_DONE_ERR));

		return(mktu_rc(tucb_ptr->header.tu, TOK_ERR,
			uncode));
	   }

	/* Check Device Driver Asynch status...
	   If status received the following elements are exepected	
	   status_s.option[0] = 0
	   status_s.option[1] = TOK_ADAP_INIT_FAIL	
	   status_s.option[2] = 0x211
	   status_s.option[3] = 0
	*/

	cc = tr_stat(fdes, &status_s, htx_sp, CIO_ASYNC_STATUS,
		START_ASYNC_ATTEMPTS, SLEEP_INTERVAL);

	/*
	 * Decrement bad_others since tr_stat incremented,
	 * remember no status is good news here.
	 */

	if (cc == 1)
	   {
		if (htx_sp != NULL)
	   		(htx_sp->bad_others)--;
	   }
	if (cc == 0)
   	   {

#ifdef debugg
	detrace(0,"tu001: We got a ASYNC_STATUS \n");
#endif 
	/* We got a CIO_ASYNC_STATUS, now we need to check 
	 * for TOK_ADAP_INIT_FAIL
	*/


	    if (status_s.option[1] == TOK_ADAP_INIT_FAIL)
	      {

#ifdef debugg 
	   	detrace(0,"tu001: We got a TOK_ADAP_INIT_FAIL \n");
#endif 		
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;

		uncode = tr_uncode(&status_s);
		if (uncode < 0)
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				ST2_DONE_ERR));
		return(mktu_rc(tucb_ptr->header.tu, TOK_ERR,
				uncode));
	      }
     	  }	
	

	/*
	 * get the network address.
	 */
	if (ioctl(fdes, IOCINFO, &devinfo_s))
	   {
#ifdef debugg
		detrace(1,"tu001:  IOCINFO FAILED with errno at %d\n", errno);
#endif
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, SYS_ERR, errno));
	   }
#ifdef debugg
	detrace(0,"tu001:  IOCINFO seemed to work...\n");
#endif


	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	/* this was eliminated because if user changes to alternate MAC
         * the frame will never be received because adapter was opened 
	 * using VPD MAC address.  net_add ( below ) is VPD address also 

         * memcpy(tucb_ptr->token_s.network_address,
         *         devinfo_s.un.token.net_addr, NETADD_LEN);
	 */

	memcpy(tucb_ptr->token_s.network_address,
		net_add, NETADD_LEN);


	return(PASSED);
   }
